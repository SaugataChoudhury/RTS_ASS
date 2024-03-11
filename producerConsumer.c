#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 10
#define N_PRODUCERS 10
#define N_CONSUMERS 10

int queue[QUEUE_SIZE];
int front = -1, rear = -1;

pthread_mutex_t mutex;
pthread_cond_t cond_producer;
pthread_cond_t cond_consumer;

pthread_t ptids[N_PRODUCERS];
pthread_t ctids[N_CONSUMERS];
int num_producers = 0;
int num_consumers = 0;

void enQ(int item);
int deQ();
void* producer(void* data);
void* consumer(void* data);
void manager();

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_producer, NULL);
    pthread_cond_init(&cond_consumer, NULL);

    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, (void*)manager, NULL);

    pthread_join(manager_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_producer);
    pthread_cond_destroy(&cond_consumer);

    return 0;
}

void enQ(int item) {
    pthread_mutex_lock(&mutex);
    if ((front == 0 && rear == QUEUE_SIZE - 1) ||
        (rear == (front - 1) % (QUEUE_SIZE - 1))) {
        printf("Queue is full, waiting for consumer to consume...\n");
        pthread_cond_wait(&cond_producer, &mutex);
    }
    else if (front == -1) {
        front = rear = 0;
        queue[rear] = item;
    }
    else if (rear == QUEUE_SIZE - 1 && front != 0) {
        rear = 0;
        queue[rear] = item;
    }
    else {
        rear++;
        queue[rear] = item;
    }
    pthread_cond_signal(&cond_consumer);
    pthread_mutex_unlock(&mutex);
}

int deQ() {
    pthread_mutex_lock(&mutex);
    if (front == -1) {
        printf("Queue is empty, waiting for producer to produce...\n");
        pthread_cond_wait(&cond_consumer, &mutex);
    }
    int item = queue[front];
    if (front == rear) {
        front = rear = -1;
    }
    else if (front == QUEUE_SIZE - 1) {
        front = 0;
    }
    else {
        front++;
    }
    pthread_cond_signal(&cond_producer);
    pthread_mutex_unlock(&mutex);
    return item;
}

void* producer(void* data) {
    int num_items = rand() % 10; 
    while (1) {
        int item = rand() % 10; 
        enQ(item);
        printf("Produced %d item: %d\n", (int) data, item);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* data) {
    while (1) {
        int item = deQ();
        printf("Consumed item: %d\n", item);
        sleep(1);
    }
    return NULL;
}

void manager() {
    while (1) {
        printf("\nSelect an option:\n");
        printf("1. Add a producer\n");
        printf("2. Remove a producer\n");
        printf("3. Add a consumer\n");
        printf("4. Remove a consumer\n");
        printf("5. Exit\n");

        int choice, count1, count2;
        scanf("%d", &choice);
		
        switch (choice) {
            case 1: {
                if (num_producers < N_PRODUCERS) {
                    pthread_create(&ptids[num_producers], NULL, producer, (void*)count1++);
                    num_producers++;
                } else {
                    printf("Cannot add more producers. Maximum limit reached.\n");
                }
                break;
            }
            case 2: {
                if (num_producers > 0) {
                    num_producers--;
                    pthread_cancel(ptids[num_producers]);
                } else {
                    printf("No producers to remove.\n");
                }
                break;
            }
            case 3: {
                if (num_consumers < N_CONSUMERS) {
                    pthread_create(&ctids[num_consumers], NULL, consumer, NULL);
                    num_consumers++;
                } else {
                    printf("Cannot add more consumers. Maximum limit reached.\n");
                }
                break;
            }
            case 4: {
                if (num_consumers > 0) {
                    num_consumers--;
                    pthread_cancel(ctids[num_consumers]);
                } else {
                    printf("No consumers to remove.\n");
                }
                break;
            }
            case 5: {
                exit(0);
            }
            default: {
                printf("Invalid choice. Please try again.\n");
                break;
            }
        }
    }
}

