#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Define task priorities
#define SENSOR_TASK_PRIORITY       (tskIDLE_PRIORITY + 2)
#define CONTROL_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)
#define SAFETY_TASK_PRIORITY       (tskIDLE_PRIORITY + 3)
#define COMMUNICATION_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

// Define task stack sizes
#define TASK_STACK_SIZE            1024

// Define threshold values
#define SENSOR_THRESHOLD           100.0f // Example threshold for sensor data
#define SAFETY_THRESHOLD           90.0f  // Example threshold for safety alert

// Function prototypes
void sensorTask(void *pvParameters);
void controlTask(void *pvParameters);
void safetyTask(void *pvParameters);
void communicationTask(void *pvParameters);

// Global variables for sensor data
float sensorData = 0.0f;

// Queues
QueueHandle_t xSensorDataQueue;

int main(void) {
    // Create queue
    xSensorDataQueue = xQueueCreate(5, sizeof(float));
    if (xSensorDataQueue == NULL) {
        printf("Failed to create sensor data queue.\n");
        exit(EXIT_FAILURE);
    }

    // Create tasks
    if (xTaskCreate(sensorTask, "Sensor Task", TASK_STACK_SIZE, NULL, SENSOR_TASK_PRIORITY, NULL) != pdPASS ||
        xTaskCreate(controlTask, "Control Task", TASK_STACK_SIZE, NULL, CONTROL_TASK_PRIORITY, NULL) != pdPASS ||
        xTaskCreate(safetyTask, "Safety Task", TASK_STACK_SIZE, NULL, SAFETY_TASK_PRIORITY, NULL) != pdPASS ||
        xTaskCreate(communicationTask, "Communication Task", TASK_STACK_SIZE, NULL, COMMUNICATION_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Failed to create one or more tasks.\n");
        exit(EXIT_FAILURE);
    }

    // Start the scheduler
    vTaskStartScheduler();

    // Should not reach here
    return 0;
}

// Task for sensor data acquisition
void sensorTask(void *pvParameters) {
    float dataToSend;
    while (1) {
        // Simulate sensor data
        dataToSend = (float)(rand() % 150);

        // Send sensor data to control task through queue
        if (xQueueSend(xSensorDataQueue, &dataToSend, portMAX_DELAY) != pdPASS) {
            printf("Failed to send sensor data to control task.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

// Task for control logic
void controlTask(void *pvParameters) {
    float receivedData;
    while (1) {
        // Receive sensor data from queue
        if (xQueueReceive(xSensorDataQueue, &receivedData, portMAX_DELAY) != pdPASS) {
            printf("Failed to receive sensor data in control task.\n");
        }

        // Check sensor data against threshold
        if (receivedData > SENSOR_THRESHOLD) {
            // Adjust plant parameters
            printf("Control Task: Adjusting plant parameters\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds
    }
}

// Task for safety monitoring
void safetyTask(void *pvParameters) {
    float receivedData;
    while (1) {
        // Receive sensor data from queue
        if (xQueueReceive(xSensorDataQueue, &receivedData, portMAX_DELAY) != pdPASS) {
            printf("Failed to receive sensor data in safety task.\n");
        }

        // Check sensor data against safety threshold
        if (receivedData > SAFETY_THRESHOLD) {
            // Trigger emergency shutdown procedures
            printf("Safety Task: Triggering emergency shutdown procedures\n");
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 5 seconds
    }
}

// Task for communication
void communicationTask(void *pvParameters) {
    while (1) {
        // Communication task logic
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay for 10 seconds
    }
}

