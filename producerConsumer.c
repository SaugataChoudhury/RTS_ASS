#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10

// Circular Queue
int queue[MAX_QUEUE_SIZE];
int front = -1, rear = -1;

// Mutex and Condition Variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consumer = PTHREAD_COND_INITIALIZER;

// Function prototypes
void enQ(int item);
int deQ();
void* producer(void* data);
void* consumer(void* data);
void manager();

int main() {
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, (void*)manager, NULL);

    pthread_join(manager_thread, NULL);

    return 0;
}

void enQ(int item) {
    pthread_mutex_lock(&mutex);
    if ((front == 0 && rear == MAX_QUEUE_SIZE - 1) ||
        (rear == (front - 1) % (MAX_QUEUE_SIZE - 1))) {
        printf("Queue is full, waiting for consumer to consume...\n");
        pthread_cond_wait(&cond_producer, &mutex);
    }
    else if (front == -1) {
        front = rear = 0;
        queue[rear] = item;
    }
    else if (rear == MAX_QUEUE_SIZE - 1 && front != 0) {
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
    else if (front == MAX_QUEUE_SIZE - 1) {
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
    int item = rand() % 100; // Generate random item
    enQ(item);
    printf("Produced item: %d\n", item);
    sleep(1); // Delay for readability
    return NULL;
}

void* consumer(void* data) {
    int item = deQ();
    printf("Consumed item: %d\n", item);
    sleep(1); // Delay for readability
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
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1: {
                pthread_t producer_thread;
                pthread_create(&producer_thread, NULL, producer, NULL);
                break;
            }
            case 2: {
                // Implement remove producer functionality
                break;
            }
            case 3: {
                pthread_t consumer_thread;
                pthread_create(&consumer_thread, NULL, consumer, NULL);
                break;
            }
            case 4: {
                // Implement remove consumer functionality
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

