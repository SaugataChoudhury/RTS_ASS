#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h> 

#define MAX_QUEUE_SIZE 10

struct shared_memory {
    int buffer[MAX_QUEUE_SIZE];
    int front, rear, count;
};

int main() {
    key_t key = ftok("producer.c", 'R');
    int shmid = shmget(key, sizeof(struct shared_memory), 0666|IPC_CREAT);
    struct shared_memory *shmem = (struct shared_memory*) shmat(shmid, NULL, 0);

    // Semaphore initialization
    int semid = semget(key, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1); // Mutex for shared memory
    semctl(semid, 1, SETVAL, 0); // Items in the queue

    // Initialize random number generator
    srand(time(NULL));

    // Add some initial items to the queue
    for (int i = 0; i < 5; i++) {
        int item = rand() % 100; // Generates a random number between 0 and 99
        shmem->buffer[shmem->rear] = item;
        shmem->rear = (shmem->rear + 1) % MAX_QUEUE_SIZE;
        shmem->count++;
    }

    while (1) {
        semop(semid, &(struct sembuf){.sem_num=0, .sem_op=-1}, 1); // Lock the shared memory
        semop(semid, &(struct sembuf){.sem_num=1, .sem_op=-1}, 1); // Wait for an empty slot in the queue

        // Generate a random number
        int item = rand() % 100; // Generates a random number between 0 and 99

        shmem->buffer[shmem->rear] = item;
        shmem->rear = (shmem->rear + 1) % MAX_QUEUE_SIZE;
        shmem->count++;

        semop(semid, &(struct sembuf){.sem_num=0, .sem_op=1}, 1); // Unlock the shared memory
    }

    shmdt(shmem);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
