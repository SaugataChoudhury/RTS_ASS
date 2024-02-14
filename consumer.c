#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define QUEUE_SIZE 10

typedef struct {
    int items[QUEUE_SIZE];
    int front, rear, count;
    sem_t *empty;
    sem_t *full;
    sem_t *mutex;
} CircularQueue;

void initQueue(CircularQueue *q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->empty = sem_open("/empty_sem", 0);
    q->full = sem_open("/full_sem", 0);
    q->mutex = sem_open("/mutex_sem", 0);
}

int consumeItem(CircularQueue *q) {
    sem_wait(q->full);
    sem_wait(q->mutex);
    int item = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->count--;
    printf("Consumed: %d\n", item);
    sem_post(q->mutex);
    sem_post(q->empty);
    return item;
}

int main() {
    CircularQueue *queue;
    int fd = shm_open("/myqueue", O_RDWR, 0666);
    queue = (CircularQueue *)mmap(NULL, sizeof(CircularQueue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    initQueue(queue);

    while (1) {
        consumeItem(queue);
        sleep(1); // Simulating some delay
    }

    return 0;
}

