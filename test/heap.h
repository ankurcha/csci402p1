/* library to provide a priority queue (heap implementation) with no 
 * dynamic memory
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef HEAP_H
#define HEAP_H 1

#include "p2pnetwork.h"
#include "packet.h"

struct heap_element {
    int key;
};
typedef struct heap_element heap_element;

struct Message {
    int key;
    int senderId;
    int senderMbox;
    Packet pkt;
};
typedef struct Message Message;

/* methods for a priority queue */

/* push the element one past the end of the heap into the heap */
void Heap_Push(Message* start, int* length);

/* remove the minimum value from the heap */
/* return value is only valid until heap is modified */
Message Heap_ExtractMin(Message* start, int* length);

#endif /* HEAP_H */

