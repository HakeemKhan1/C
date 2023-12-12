//Hakeem Khan 101233039
#include "header.h"

int main(){
    //SETUP
    int messageQueueId; // ID of message queue for communication
    int semaphoreSharedKey, semaphoreNotEmptyKey, semaphoreEmptyKey, sharedMemoryKey; // Keys for semaphores and shared memory
    int semaphoreSharedId, semaphoreNotEmptyId, semaphoreEmptyId, sharedMemoryId; // IDs for semaphores and shared memory
    int fileSize; // Size of the input file
    int currentBufferIndex = 0; // Current index for writing to the buffer
    int previousSequenceNumber = 0; // Previous sequence number
    struct buffers *sharedBuffers; // Pointer to access shared buffers
    struct my_msg_st resourceRequest; // Message structure for requesting and receiving keys
    struct my_msg_st fileSizeMessage; // Message struct for sending file size
    struct stat fileStat; // Struct to get file size
    void *sharedMemoryPointer = (void*)0; // Pointer to the shared memory segment
    char bufferData[1024]; // Buffer for reading data from the file
    FILE *filePointer; // Pointer to the input file
    
    // Creating message queue
    messageQueueId = msgget((key_t)MQUEUE, 0666 | IPC_CREAT);
    if (messageQueueId == -1) {
        fprintf(stderr, "Message queue creation failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Requesting resource keys
    sharedMemoryKey = request_and_receive(messageQueueId, "shmid", resourceRequest, getpid());
    semaphoreSharedKey = request_and_receive(messageQueueId, "semS", resourceRequest, getpid());
    semaphoreNotEmptyKey = request_and_receive(messageQueueId, "semN", resourceRequest, getpid());
    semaphoreEmptyKey = request_and_receive(messageQueueId, "semE", resourceRequest, getpid());
    
    // Creating and initializing semaphores
    semaphoreSharedId = semget((key_t)semaphoreSharedKey, 1, 0666 | IPC_CREAT);
    semaphoreNotEmptyId = semget((key_t)semaphoreNotEmptyKey, 1, 0666 | IPC_CREAT);
    semaphoreEmptyId = semget((key_t)semaphoreEmptyKey, 1, 0666 | IPC_CREAT);
    if (!set_semvalue(semaphoreSharedId, 1)) {
        fprintf(stderr, "Initialization of semaphore for shared access failed\n");
        exit(EXIT_FAILURE);
    }
    if (!set_semvalue(semaphoreNotEmptyId, 0)) {
        fprintf(stderr, "Initialization of semaphore for non-empty condition failed\n");
        exit(EXIT_FAILURE);
    }
    if (!set_semvalue(semaphoreEmptyId, NUM_BUFFERS)) {
        fprintf(stderr, "Initialization of semaphore for empty condition failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Semaphores for shared access, non-empty, and empty conditions have been initialized\n\n");

    // Setting up shared memory
    sharedMemoryId = shmget((key_t)sharedMemoryKey, sizeof(struct buffers), 0666 | IPC_CREAT);
    if (sharedMemoryId == -1) {
        fprintf(stderr, "Shared memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    sharedMemoryPointer = shmat(sharedMemoryId, (void *)0, 0);
    if (sharedMemoryPointer == (void *)-1) {
        fprintf(stderr, "Shared memory attachment failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory successfully attached\n");
    sharedBuffers = (struct buffers *)sharedMemoryPointer; 

    // Opening file and getting the file size
    filePointer = fopen(INPUT_FILE, "r");
    if (filePointer == NULL) {
        fprintf(stderr, "Failed to open input file\n");
        exit(EXIT_FAILURE);
    }
    stat(INPUT_FILE, &fileStat);
    fileSize = fileStat.st_size;
    printf("Size of the input file: %d bytes\n\n", fileSize);

    // Sending the file size to the consumer with message type 3
    sprintf(fileSizeMessage.some_text, "%d", fileSize);
    fileSizeMessage.my_msg_type = 3;
    if (msgsnd(messageQueueId, (void *)&fileSizeMessage, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "Failed to send file size to consumer\n");
        exit(EXIT_FAILURE);
    }

    // Reading from input file and writing to shared buffer
    while (fgets(bufferData, BUFFER_SIZE, filePointer) != NULL) {
	    
	    if (!semaphore_p(semaphoreEmptyId)) exit(EXIT_FAILURE);// Acquire semaphore to ensure there is space in the buffer
	    
	    if (!semaphore_p(semaphoreSharedId)) exit(EXIT_FAILURE);// Acquire semaphore to access the shared buffer

	    
	    strcpy(sharedBuffers->buffer[currentBufferIndex].data, bufferData);// Copy data from the file buffer to the shared buffer
	   
	    sharedBuffers->buffer[currentBufferIndex].seq_num = previousSequenceNumber;// Set the sequence number for the current buffer
	    
	    sharedBuffers->buffer[currentBufferIndex].count = strlen(bufferData);// Set the count of bytes in the current buffer

	    // Print information about the current buffer
	    printf("%d. buffer[%d] sequence number: %d\n", currentBufferIndex, currentBufferIndex, sharedBuffers->buffer[currentBufferIndex].seq_num);
	    printf("%d. buffer[%d] bytes: %d\n", currentBufferIndex, currentBufferIndex, sharedBuffers->buffer[currentBufferIndex].count);

	    
	    if (!semaphore_v(semaphoreSharedId)) exit(EXIT_FAILURE);// Release semaphore to signal that the shared buffer is updated
	    
	    if (!semaphore_v(semaphoreNotEmptyId)) exit(EXIT_FAILURE);// Release semaphore to signal that the buffer is not empty

	    
	    currentBufferIndex = (currentBufferIndex + 1) % NUM_BUFFERS;// Update the index for the current buffer and sequence number
	    previousSequenceNumber += strlen(bufferData);
    }

    printf("Producer finished reading file\n");
    fclose(filePointer);

    // Deallocating semaphores and shared memory
    sleep(2); // Waiting for consumer to finish
    printf("Starting resource deallocation\nTHANK YOU!\n");
    del_semvalue(semaphoreSharedId);
    del_semvalue(semaphoreNotEmptyId);
    del_semvalue(semaphoreEmptyId);
    if (shmdt(sharedMemoryPointer) == -1) {
        fprintf(stderr, "Failed to detach shared memory\n");
        exit(EXIT_FAILURE);
    }
    if (shmctl(sharedMemoryId, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Failed to delete shared memory\n");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

