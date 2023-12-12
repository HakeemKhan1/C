//Hakeem Khan 101233039
#include "header.h"

int main() {

    //SETUP
    
    struct my_msg_st requestMsg;
    struct buffers *sharedBuffers; // Pointer to access the buffers in shared memory
    int messageQueueId; // ID of the message queue for receiving and sending messages
    int shmKey, semSKey, semNKey, semEKey;
    int semSId, semNId, semEId, shmId; 
    int fileSize; // Size of the file to be read
    int currentBufferIndex = 0; // Index for reading data from the shared buffer
    int expectedSequenceNum = 0; // Expected sequence number for data validation
    char bufferData[1024]; // Buffer for reading data from shared memory
    void *sharedMemPtr = (void *)0; // Pointer for attaching the shared memory segment  
    FILE *outputFilePtr; // File pointer for the output file
    
    // Initialize message queue
    messageQueueId = msgget((key_t)MQUEUE, 0666 | IPC_CREAT);
    if (messageQueueId == -1) {
        fprintf(stderr, "Error creating message queue: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Request and receive shared memory and semaphore keys
    shmKey = request_and_receive(messageQueueId, "shmid", requestMsg, getpid());
    semSKey = request_and_receive(messageQueueId, "semS", requestMsg, getpid());
    semNKey = request_and_receive(messageQueueId, "semN", requestMsg, getpid());
    semEKey = request_and_receive(messageQueueId, "semE", requestMsg, getpid());
    
    // Acquire semaphore IDs
    semSId = semget((key_t)semSKey, 1, 0666 | IPC_CREAT);
    semNId = semget((key_t)semNKey, 1, 0666 | IPC_CREAT);
    semEId = semget((key_t)semEKey, 1, 0666 | IPC_CREAT);

    // Set up shared memory
    printf("Set up shared memory for buffers\n");
    shmId = shmget((key_t)shmKey, sizeof(struct buffers), 0666 | IPC_CREAT);
    if (shmId == -1) {
        fprintf(stderr, "Error acquiring shared memory\n");
        exit(EXIT_FAILURE);
    }
    sharedMemPtr = shmat(shmId, (void *)0, 0);
    if (sharedMemPtr == (void *)-1) {
        fprintf(stderr, "Error attaching shared memory\n");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory successful\n");
    sharedBuffers = (struct buffers *)sharedMemPtr;

    // Receive file size from the producer
    if (msgrcv(messageQueueId, (void *)&requestMsg, BUFSIZ, 3, 0) == -1) {
        fprintf(stderr, "Error receiving message: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    fileSize = atoi(requestMsg.some_text);
    printf("Received file size from producer: %d\n", fileSize);

    // Open output file
    outputFilePtr = fopen(OUTPUT_FILE, "w");
    if (outputFilePtr == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(EXIT_FAILURE);
    }

    // Read from buffer and write to output file
    while (1) {
        if (!semaphore_p(semNId)) exit(EXIT_FAILURE); // Acquire full semaphore
        if (!semaphore_p(semSId)) exit(EXIT_FAILURE); // Acquire mutex semaphore

        strcpy(bufferData, sharedBuffers->buffer[currentBufferIndex].data); // Read data
        fputs(bufferData, outputFilePtr); fflush(outputFilePtr); // Write data to file
        if (sharedBuffers->buffer[currentBufferIndex].seq_num != expectedSequenceNum) { 
            printf("Sequence number mismatch: %d, expected: %d\n", sharedBuffers->buffer[currentBufferIndex].seq_num, expectedSequenceNum);
        }

        if (!semaphore_v(semSId)) exit(EXIT_FAILURE); // Release mutex semaphore
        if (!semaphore_v(semEId)) exit(EXIT_FAILURE); // Release empty semaphore
        expectedSequenceNum += strlen(bufferData);
        currentBufferIndex = (currentBufferIndex + 1) % NUM_BUFFERS;
        if (expectedSequenceNum >= fileSize) {
            break;
        }
    }

    // Verify file size consistency
    if (expectedSequenceNum == fileSize) {
        printf("File size verified: %d bytes\n", fileSize);
    } else {
        printf("File size mismatch: expected %d, read %d bytes\n", fileSize, expectedSequenceNum);
    }

    // Cleanup and exit
    printf("Closing output file\nTHANK YOU!\n");
    fclose(outputFilePtr);
    exit(EXIT_SUCCESS);
}

