#include "header.h"

int main() {
    int serverQueueId;
    struct my_msg_st receivedData;
    struct my_msg_st sendData;
    char clientRequest[1000];
    char responseMessage[2000];
    int isRunning = 1;
    int checker = 0;
    int key;
    int semSKey, semNKey, semEKey, shMemKey;
    struct buffers sharedBuffers;

    srand((unsigned int)time(NULL));
    // generate random keys between 1000 and 9999
    semSKey = rand() % 9000 + 1000;
    semNKey = rand() % 9000 + 1000;
    semEKey = rand() % 9000 + 1000;
    shMemKey = rand() % 9000 + 1000;

    // Initialize shared buffer structures
    for (int i = 0; i < NUM_BUFFERS; i++) {
        struct buffer newBuffer;
        sharedBuffers.buffer[i] = newBuffer;
    }

    // Create the message queue 
    serverQueueId = msgget((key_t)MQUEUE, 0666 | IPC_CREAT);
    if (serverQueueId == -1) {
        fprintf(stderr, "Message queue creation failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    // Server main loop
    while (isRunning) {
    	
        if (msgrcv(serverQueueId, (void *)&receivedData, BUFSIZ, 1, 0) == -1) {
            fprintf(stderr, "Message receive failed with error: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        
        printf("Message Received: %s is the message\n", receivedData.some_text);  
        sscanf(receivedData.some_text, "REQUEST;%s", clientRequest);
        printf("Clients Request: %s is the request\n", clientRequest);  
        // Determine key based on request
        
            // Check if the client request corresponds to the semaphore 'semS'
	    if (strcmp(clientRequest, "semS") == 0) {
		key = semSKey; // Set the key to the semaphore 'semS' key
	    } 
	    // Check if the client request corresponds to the semaphore 'semN'
	    else if (strcmp(clientRequest, "semN") == 0) {
		key = semNKey; // Set the key to the semaphore 'semN' key
	    } 
	    // Check if the client request corresponds to the semaphore 'semE'
	    else if (strcmp(clientRequest, "semE") == 0) {
		key = semEKey; // Set the key to the semaphore 'semE' key
	    } 
	    // If the client request does not match any known semaphore, set the key to the shared memory key
	    else {
		key = shMemKey; // Set the key to the shared memory key
	    }

        
        // Prepare and send response message
        sprintf(responseMessage, "RESPONSE;%s;%d", clientRequest, key);
        sendData.my_msg_type = receivedData.pid;
        strcpy(sendData.some_text, responseMessage);
        if (msgsnd(serverQueueId, (void *)&sendData, BUFSIZ, 0) == -1) {
            fprintf(stderr, "Message send failed\n");
            exit(EXIT_FAILURE);
        } else {
            printf("Response back to client: %ld is the message\n\n", sendData.my_msg_type);
        }
        
        //End the server after the last semaphore
        if (strcmp(clientRequest, "semE") == 0) {
            checker +=1; 
            if (checker == 2){ //check for the second semaphore E to be requested
            	sleep(4);
            	isRunning =0;
            }
        }
    }

    // Destroy message queue
    if (msgctl(serverQueueId, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Message queue cleanup failed\n");
        exit(EXIT_FAILURE);
    } 
}

