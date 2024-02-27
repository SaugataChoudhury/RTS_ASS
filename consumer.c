#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

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

    while (1) {
        semop(semid, &(struct sembuf){.sem_num=0, .sem_op=-1}, 1); // Lock the shared memory
        semop(semid, &(struct sembuf){.sem_num=1, .sem_op=-1}, 1); // Wait for an item in the queue

        int item = shmem->buffer[shmem->front];
        shmem->front = (shmem->front + 1) % MAX_QUEUE_SIZE;
        shmem->count--;

        semop(semid, &(struct sembuf){.sem_num=0, .sem_op=1}, 1); // Unlock the shared memory

        printf("Consumed item: %d\n", item);
    }

    shmdt(shmem);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
