/* library to provide a queue data structure with no dynamic memory
 *
 * CS 402 Fall 2009
 * Max Pflueger
 */

#include "print.h"
#include "queue.h"

void Init_Queue(Queue* q) {
    int i = 0;

    q->head = -1;
    q->tail = -1;

    while (i < q->length) {
        q->queue[i].next = -1;
        q->queue[i].valid = 0;
        i++;
    }

    return;
}

void Queue_Push(Queue* q, int value) {
    int i = 0;

    /* find an open slot */
    while (i < q->length) {
        if (!q->queue[i].valid) {
            break;
        }
        i++;
    }

    /* verify an open slot was found */
    if (i == q->length) {
        print("ERROR: queue ran out of space!\n");
        return;
    }

    /* put our new element in the slot */
    if (q->tail == -1) {
        /* the list was empty */
        q->head = i;
    } else {
        q->queue[q->tail].next = i;
    }
    q->queue[i].valid = 1;
    q->queue[i].next = -1;
    q->queue[i].value = value;

    q->tail = i;

    return;
}

int Queue_Pop(Queue* q) {
    int temp;

    /* check that there is something to pop */
    if (q->head == -1) {
        print("ERROR: no element in queue to pop!\n");
        return -1;
    }

    /* remove the top element from the queue */
    temp = q->head;

    /* check if we popped the last element */
    if (q->tail == q->head) {
        q->head = -1;
        q->tail = -1;
    } else {
        q->head = q->queue[q->head].next;
        if (q->tail == -1) {
            print("ERROR: queue is malformed!\n");
        }
    }

    q->queue[temp].valid = 0;
    q->queue[temp].next = -1;

    return q->queue[temp].value;
}

char Queue_IsEmpty(Queue* q) {
    if (q->head == -1) {
        if (q->tail != -1) {
            print("ERROR: queue is headless! run for your lives!\n");
        }
        return 1;
    } else {
        return 0;
    }
}

