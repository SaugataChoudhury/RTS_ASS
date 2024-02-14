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
    q->empty = sem_open("/empty_sem", O_CREAT, 0666, QUEUE_SIZE);
    q->full = sem_open("/full_sem", O_CREAT, 0666, 0);
    q->mutex = sem_open("/mutex_sem", O_CREAT, 0666, 1);
}

void produceItem(CircularQueue *q, int item) {
    sem_wait(q->empty);
    sem_wait(q->mutex);
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->items[q->rear] = item;
    q->count++;
    printf("Produced: %d\n", item);
    sem_post(q->mutex);
    sem_post(q->full);
}

int main() {
    int item;
    CircularQueue *queue;
    int fd = shm_open("/myqueue", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(CircularQueue));
    queue = (CircularQueue *)mmap(NULL, sizeof(CircularQueue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    initQueue(queue);

    while (1) {
        printf("Enter an integer: ");
        scanf("%d", &item);
        produceItem(queue, item);
        sleep(1); // Simulating some delay
    }

    return 0;
}

