/* library to provide a queue data structure with no dynamic memory
 *
 * CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef QUEUE_H
#define QUEUE_H 1

struct queue_element {
	int value;
	int next; /* must init to -1 */
	char valid; /* must init to 0 */
};

typedef struct queue_element queue_element;

struct Queue {
	queue_element* queue;
	int length; /* must init to the length of the array */
	int head; /* must init to -1 */
	int tail; /* must init to -1 */
};

typedef struct Queue Queue;

/* methods for a FIFO queue */

/* Initialize values of the queue */
void Init_Queue(Queue* q);

/* Push a value to the end of the queue 
 *  Note that the first arg must be an array of length n*/
void Queue_Push(Queue* q, int value);

/* Pop a value off the top of the queue */
int Queue_Pop(Queue* q);

/* Check if the queue is empty */
char Queue_IsEmpty(Queue* q);

#endif /* QUEUE_H */
