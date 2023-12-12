//Hakeem Khan 101233039
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/stat.h>

#define MQUEUE 1234
#define MAX_TEXT 512
#define NUM_BUFFERS 10
#define BUFFER_SIZE 1024
#define INPUT_FILE "Story.txt"
#define OUTPUT_FILE "Story_output.txt"

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif
struct my_msg_st {
    long int my_msg_type;
    pid_t pid;
    char some_text[BUFSIZ];
};

struct buffer{
    char data[BUFFER_SIZE];
    int seq_num;
    int count;
};

struct buffers{
    struct buffer buffer[NUM_BUFFERS];
};

int request_and_receive(int msgid, char *req_msg, struct my_msg_st request_struct, pid_t pid){
    int key;
    sprintf(request_struct.some_text, "REQUEST;%s", req_msg);
    request_struct.my_msg_type = 1; // Direction 1: client to server, and client's PID for server to client communication
    request_struct.pid = pid;
    if (msgsnd(msgid, (void *)&request_struct, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Request sent to server for %s\n", request_struct.some_text);
    }
    if (msgrcv(msgid, (void *)&request_struct, BUFSIZ, pid, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    else {
        printf("Response received: %s\n", request_struct.some_text);
        sscanf(request_struct.some_text, "RESPONSE;%[^;];%d", request_struct.some_text, &key);
        printf("Key is: %d\n\n", key);
        return key;
    }
}
static int set_semvalue(int id, int val){
    union semun sem_union;

    sem_union.val = val;
    if (semctl(id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

static void del_semvalue(int id){
    union semun sem_union;
    
    if (semctl(id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int id){
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = 0;
    if (semop(id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

static int semaphore_v(int id){
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = 0;
    if (semop(id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}





