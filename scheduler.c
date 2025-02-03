#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

Queue priorityQueues[MAX_PRIORITY + 1] = {{0}};
int samePriorityCounter[MAX_PRIORITY + 1] = {0};

void appendItem(Queue *queue, void *data) {
    if (queue == NULL) {  // Check if queue is NULL
        return;  // Exit early if queue is NULL
    }

    QueueItem *newItem = (QueueItem *)malloc(sizeof(QueueItem));
    if (newItem == NULL) {  // Check if memory allocation failed
        return;  // Exit early if memory allocation failed
    }

    newItem->data = data;
    newItem->next = NULL;

    if (queue->head == NULL) {
        queue->head = newItem;
    } else {
        queue->tail->next = newItem;
        queue->tail = newItem;
    }

}

void *removeFirstItem(Queue *queue) {
    if (queue == NULL) {  // Check if queue is NULL
        return NULL;  // Exit early if queue is NULL
    }
    if (queue->head == NULL) {  // Check if queue is empty
        return NULL;  // Exit early if queue is empty
    }

    QueueItem *head = queue->head;
    Task *data = head->data;
    queue->head = head->next;
    free(head);
    return data;
}

void initScheduler() {
    for (int i = 0; i <= MAX_PRIORITY; i++) {
        priorityQueues[i].head = NULL;
        priorityQueues[i].tail = NULL;
        samePriorityCounter[i] = 0;
    }
}

void onTaskReady(Task *task) {
    if (task == NULL || task->state != STATE_WAITING) {
        return;  // Only transition from WAITING to READY
    }

    task->state = STATE_READY;
    appendItem(&priorityQueues[task->priority], task);
}

void onTaskPreempted(Task *task) {
    if (task == NULL || task->state != STATE_RUNNING) {
        return;  // Only transition from RUNNING to READY
    }

    task->state = STATE_READY;
    appendItem(&priorityQueues[task->priority], task);
}

void onTaskWaiting(Task *task) {
    if (task == NULL || task->state != STATE_RUNNING) {
        return;  // Only transition from RUNNING to WAITING
    }

    task->state = STATE_WAITING;
}

static Task *_scheduleNextTaskRecursive(int priority) {
    if (priority > MAX_PRIORITY) {
        return NULL;  // No tasks found in any priority queue
    }

    if (priorityQueues[priority].head != NULL) {
        if (samePriorityCounter[priority] < MAX_SAMEPRIO_LENGTH) {
            // Schedule a task from the current priority queue
            Task *task = (Task *)removeFirstItem(&priorityQueues[priority]);
            if (task == NULL) {
                return _scheduleNextTaskRecursive(priority + 1);  // Skip if task is NULL
            }

            if (task->state != STATE_READY) {
                return _scheduleNextTaskRecursive(priority + 1);  // Skip if task is not READY
            }

            task->state = STATE_RUNNING;
            samePriorityCounter[priority]++;  // Increment the counter for this priority

            // Reset counters for lower priorities
            for (int j = priority + 1; j <= MAX_PRIORITY; j++) {
                samePriorityCounter[j] = 0;
            }

            return task;  // Successfully scheduled a task
        } else {
            // Reset the counter for the current priority and try the next lower priority
            samePriorityCounter[priority] = 0;
            return _scheduleNextTaskRecursive(priority + 1);
        }
    }

    // No tasks in the current priority queue, try the next lower priority
    return _scheduleNextTaskRecursive(priority + 1);
}

    

Task *scheduleNextTask() {
    return _scheduleNextTaskRecursive(0);  // Start with the highest priority (0)
}
