/*#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_PRODUCERS 10
#define MAX_CONSUMERS 10

int queue[MAX_QUEUE_SIZE];
int front = -1, rear = -1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consumer = PTHREAD_COND_INITIALIZER;

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
    while (1) {
        int num_items = rand() % 10 + 1; // Generate random number of items to produce
        for (int i = 0; i < num_items; i++) {
            int item = rand() % 100; // Generate random item
            enQ(item);
            printf("Produced item: %d\n", item);
            sleep(1); // Delay for readability
        }
    }
    return NULL;
}

void* consumer(void* data) {
    while (1) {
        int num_items = rand() % 10 + 1; // Generate random number of items to consume
        for (int i = 0; i < num_items; i++) {
            int item = deQ();
            printf("Consumed item: %d\n", item);
            sleep(1); // Delay for readability
        }
    }
    return NULL;
}

void manager() {
    int num_producers = 0, num_consumers = 0;
    pthread_t producers[MAX_PRODUCERS];
    pthread_t consumers[MAX_CONSUMERS];

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
                if (num_producers < MAX_PRODUCERS) {
                    pthread_create(&producers[num_producers], NULL, producer, NULL);
                    num_producers++;
                } else {
                    printf("Cannot add more producers. Maximum limit reached.\n");
                }
                break;
            }
            case 2: {
                if (num_producers > 0) {
                    num_producers--;
                    pthread_cancel(producers[num_producers]);
                } else {
                    printf("No producers to remove.\n");
                }
                break;
            }
            case 3: {
                if (num_consumers < MAX_CONSUMERS) {
                    pthread_create(&consumers[num_consumers], NULL, consumer, NULL);
                    num_consumers++;
                } else {
                    printf("Cannot add more consumers. Maximum limit reached.\n");
                }
                break;
            }
            case 4: {
                if (num_consumers > 0) {
                    num_consumers--;
                    pthread_cancel(consumers[num_consumers]);
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

int main() {
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, (void*)manager, NULL);

    pthread_join(manager_thread, NULL);

    return 0;
}










#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 10
#define MAX_ITEMS 10

typedef struct {
    int items[QUEUE_SIZE];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full, not_empty;
} CircularQueue;

typedef struct {
    pthread_t thread;
    int active;
} ThreadInfo;

CircularQueue queue = { .front = 0, .rear = -1, .count = 0,
                        .mutex = PTHREAD_MUTEX_INITIALIZER,
                        .not_full = PTHREAD_COND_INITIALIZER,
                        .not_empty = PTHREAD_COND_INITIALIZER };

ThreadInfo producers[10];
ThreadInfo consumers[10];
int num_producers = 0;
int num_consumers = 0;

void enQ(int item) {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count >= QUEUE_SIZE) {
        pthread_cond_wait(&queue.not_full, &queue.mutex);
    }
    queue.rear = (queue.rear + 1) % QUEUE_SIZE;
    queue.items[queue.rear] = item;
    queue.count++;
    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
}

int deQ() {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count <= 0) {
        pthread_cond_wait(&queue.not_empty, &queue.mutex);
    }
    int item = queue.items[queue.front];
    queue.front = (queue.front + 1) % QUEUE_SIZE;
    queue.count--;
    pthread_cond_signal(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
    return item;
}

int noOfFreeSpacesQ() {
    pthread_mutex_lock(&queue.mutex);
    int free_spaces = QUEUE_SIZE - queue.count;
    pthread_mutex_unlock(&queue.mutex);
    return free_spaces;
}

void* producer(void *data) {
    while (1) {
        int num_items = rand() % MAX_ITEMS + 1;
        pthread_mutex_lock(&queue.mutex);
        while (noOfFreeSpacesQ() < num_items) {
            pthread_cond_wait(&queue.not_full, &queue.mutex);
        }
        pthread_mutex_unlock(&queue.mutex);

        for (int i = 0; i < num_items; i++) {
            int item = rand() % 100;
            enQ(item);
            printf("Produced: %d\n", item);
        }
        usleep(rand() % 1000000);
    }
    return NULL;
}

void* consumer(void *data) {
    while (1) {
        int num_items = rand() % MAX_ITEMS + 1;
        pthread_mutex_lock(&queue.mutex);
        while (queue.count < num_items) {
            pthread_cond_wait(&queue.not_empty, &queue.mutex);
        }
        pthread_mutex_unlock(&queue.mutex);

        for (int i = 0; i < num_items; i++) {
            int item = deQ();
            printf("Consumed: %d\n", item);
        }
        usleep(rand() % 1000000);
    }
    return NULL;
}

void* manager(void *data) {
    while (1) {
        printf("\n1. Add producer thread\n2. Add consumer thread\n3. Delete producer thread\n4. Delete consumer thread\n5. Exit\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                pthread_create(&producers[num_producers].thread, NULL, producer, NULL);
                producers[num_producers].active = 1;
                num_producers++;
                printf("Producer thread added.\n");
                break;
            case 2:
                pthread_create(&consumers[num_consumers].thread, NULL, consumer, NULL);
                consumers[num_consumers].active = 1;
                num_consumers++;
                printf("Consumer thread added.\n");
                break;
            case 3:
                if (num_producers > 0) {
                    num_producers--;
                    producers[num_producers].active = 0;
                    pthread_cancel(producers[num_producers].thread);
                    printf("Producer thread deleted.\n");
                } else {
                    printf("No active producer threads to delete.\n");
                }
                break;
            case 4:
                if (num_consumers > 0) {
                    num_consumers--;
                    consumers[num_consumers].active = 0;
                    pthread_cancel(consumers[num_consumers].thread);
                    printf("Consumer thread deleted.\n");
                } else {
                    printf("No active consumer threads to delete.\n");
                }
                break;
            case 5:
                printf("Exiting...\n");
                pthread_exit(NULL);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
    return NULL;
}

int main() {
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, manager, NULL);

    pthread_join(manager_thread, NULL);

    return 0;
}


*/










/*
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 10
#define MAX_ITEMS 10

typedef struct {
    int items[QUEUE_SIZE];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full, not_empty;
} CircularQueue;

typedef struct {
    pthread_t thread;
    int active;
} ThreadInfo;

CircularQueue queue = { .front = 0, .rear = -1, .count = 0,
                        .mutex = PTHREAD_MUTEX_INITIALIZER,
                        .not_full = PTHREAD_COND_INITIALIZER,
                        .not_empty = PTHREAD_COND_INITIALIZER };

ThreadInfo producers[10];
ThreadInfo consumers[10];
int num_producers = 0;
int num_consumers = 0;

void enQ(int item) {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count >= QUEUE_SIZE) {
        pthread_cond_wait(&queue.not_full, &queue.mutex);
    }
    queue.rear = (queue.rear + 1) % QUEUE_SIZE;
    queue.items[queue.rear] = item;
    queue.count++;
    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
}

int deQ() {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count <= 0) {
        pthread_cond_wait(&queue.not_empty, &queue.mutex);
    }
    int item = queue.items[queue.front];
    queue.front = (queue.front + 1) % QUEUE_SIZE;
    queue.count--;
    pthread_cond_signal(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
    return item;
}

int noOfFreeSpacesQ() {
    pthread_mutex_lock(&queue.mutex);
    int free_spaces = QUEUE_SIZE - queue.count;
    pthread_mutex_unlock(&queue.mutex);
    return free_spaces;
}

void* producer(void *data) {
    while (1) {
        int num_items = rand() % MAX_ITEMS + 1;
        pthread_mutex_lock(&queue.mutex);
        while (noOfFreeSpacesQ() < num_items) {
            pthread_cond_wait(&queue.not_full, &queue.mutex);
        }
        pthread_mutex_unlock(&queue.mutex);

        for (int i = 0; i < num_items; i++) {
            int item = rand() % 100;
            enQ(item);
            printf("Produced: %d\n", item);
        }
        usleep(rand() % 1000000);
    }
    return NULL;
}

void* consumer(void *data) {
    while (1) {
        int num_items = rand() % MAX_ITEMS + 1;
        pthread_mutex_lock(&queue.mutex);
        while (queue.count < num_items) {
            pthread_cond_wait(&queue.not_empty, &queue.mutex);
        }
        pthread_mutex_unlock(&queue.mutex);

        for (int i = 0; i < num_items; i++) {
            int item = deQ();
            printf("Consumed: %d\n", item);
        }
        usleep(rand() % 1000000);
    }
    return NULL;
}

void* manager(void *data) {
    while (1) {
        printf("\n1. Add producer thread\n2. Add consumer thread\n3. Delete producer thread\n4. Delete consumer thread\n5. Exit\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                pthread_create(&producers[num_producers].thread, NULL, producer, NULL);
                producers[num_producers].active = 1;
                num_producers++;
                printf("Producer thread added.\n");
                break;
            case 2:
                pthread_create(&consumers[num_consumers].thread, NULL, consumer, NULL);
                consumers[num_consumers].active = 1;
                num_consumers++;
                printf("Consumer thread added.\n");
                break;
            case 3:
                if (num_producers > 0) {
                    num_producers--;
                    producers[num_producers].active = 0;
                    pthread_cancel(producers[num_producers].thread);
                    printf("Producer thread deleted.\n");
                } else {
                    printf("No active producer threads to delete.\n");
                }
                break;
            case 4:
                if (num_consumers > 0) {
                    num_consumers--;
                    consumers[num_consumers].active = 0;
                    pthread_cancel(consumers[num_consumers].thread);
                    printf("Consumer thread deleted.\n");
                } else {
                    printf("No active consumer threads to delete.\n");
                }
                break;
            case 5:
                printf("Exiting...\n");
                pthread_exit(NULL);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
    return NULL;
}

int main() {
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, manager, NULL);

    pthread_join(manager_thread, NULL);

    return 0;
}








*/





#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_PRODUCERS 10
#define MAX_CONSUMERS 10

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
    if ((rear + 1) % MAX_QUEUE_SIZE == front) {
        printf("Queue is full, waiting for consumer to consume...\n");
        pthread_cond_wait(&cond_producer, &mutex);
    }
    else if (front == -1) {
        front = rear = 0;
        queue[rear] = item;
    }
    else {
        rear = (rear + 1) % MAX_QUEUE_SIZE;
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
    else {
        front = (front + 1) % MAX_QUEUE_SIZE;
    }
    pthread_cond_signal(&cond_producer);
    pthread_mutex_unlock(&mutex);
    return item;
}

void* producer(void* data) {
    int num_items = rand() % 10 + 1; // Generate random number of items (1 to 10)
    for (int i = 0; i < num_items; i++) {
        int item = rand() % 100; // Generate random item
        enQ(item);
        printf("Produced item: %d\n", item);
        sleep(1); // Delay for readability
    }
    return NULL;
}

void* consumer(void* data) {
    while (1) {
        int item = deQ();
        printf("Consumed item: %d\n", item);
        sleep(1); // Delay for readability
    }
    return NULL;
}

void manager() {
    pthread_t producers[MAX_PRODUCERS];
    pthread_t consumers[MAX_CONSUMERS];
    int num_producers = 0;
    int num_consumers = 0;

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
                if (num_producers < MAX_PRODUCERS) {
                    pthread_create(&producers[num_producers], NULL, producer, NULL);
                    num_producers++;
                } else {
                    printf("Cannot add more producers. Maximum limit reached.\n");
                }
                break;
            }
            case 2: {
                if (num_producers > 0) {
                    num_producers--;
                    pthread_cancel(producers[num_producers]);
                } else {
                    printf("No producers to remove.\n");
                }
                break;
            }
            case 3: {
                pthread_create(&consumers[num_consumers], NULL, consumer, NULL);
                num_consumers++;
                break;
            }
            case 4: {
                if (num_consumers > 0) {
                    num_consumers--;
                    pthread_cancel(consumers[num_consumers]);
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

