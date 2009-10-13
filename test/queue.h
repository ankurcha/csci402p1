/* library to provide a queue data structure with no dynamic memory
 *
 * CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef QUEUE_H
#define QUEUE_H

struct queue_element{
    int value;
    int next = -1;
    char valid = 0;
};

typedef struct queue_element queue_element;

struct Queue{
    queue_element* queue;
    int length = -1;
    int head = -1;
    int tail = -1;
};

typedef struct Queue Queue;

/* methods for a FIFO queue */

/* Push a value to the end of the queue 
 *  Note that the first arg must be an array of length n*/
void Queue_Push(Queue* queue, int value);

/* Pop a value off the top of the queue */
int Queue_Pop(Queue* queue);

#endif /* QUEUE_H */
