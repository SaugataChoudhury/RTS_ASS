#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHARED_MEMORY_KEY IPC_PRIVATE 

// Function to compute product of a row in matrix A and column in matrix B
void computeRowColumn(int row, int col, int *a, int *b, int *c, int m, int p) {
    int sum = 0;
    for (int i = 0; i < m; i++) {
        sum += a[row * m + i] * b[i * p + col];
    }
    c[row * p + col] = sum;
}

int main() {
    int shmid, rows_A, cols_A, cols_B;
    int *a, *b, *c;

    // Read dimensions of matrix A
    printf("Enter the number of rows and columns for matrix A: ");
    scanf("%d %d", &rows_A, &cols_A);

    // Read dimensions of matrix B
    printf("Enter the number of columns for matrix B: ");
    scanf("%d", &cols_B);

    // Create shared memory for matrices A, B, and C
    shmid = shmget(SHARED_MEMORY_KEY, sizeof(int) * (rows_A * cols_A + cols_A * cols_B + rows_A * cols_B), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory
    int *shared_memory = (int *)shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Pointers to matrices A, B, and C
    a = shared_memory;
    b = a + rows_A * cols_A;
    c = b + cols_A * cols_B;

    // Read matrix A from user
    printf("Enter elements of matrix A (%d x %d):\n", rows_A, cols_A);
    for (int i = 0; i < rows_A; i++) {
        for (int j = 0; j < cols_A; j++) {
            scanf("%d", &a[i * cols_A + j]);
        }
    }

    // Read matrix B from user
    printf("Enter elements of matrix B (%d x %d):\n", cols_A, cols_B);
    for (int i = 0; i < cols_A; i++) {
        for (int j = 0; j < cols_B; j++) {
            scanf("%d", &b[i * cols_B + j]);
        }
    }

    // Fork child processes to compute rows of matrix C
    for (int i = 0; i < rows_A; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            for (int j = 0; j < cols_B; j++) {
                computeRowColumn(i, j, a, b, c, cols_A, cols_B);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < rows_A; i++) {
        wait(NULL);
    }

    // Print product matrix C
    printf("Product matrix C (%d x %d):\n", rows_A, cols_B);
    for (int i = 0; i < rows_A; i++) {
        for (int j = 0; j < cols_B; j++) {
            printf("%d ", c[i * cols_B + j]);
        }
        printf("\n");
    }

    // Detach and release shared memory
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

