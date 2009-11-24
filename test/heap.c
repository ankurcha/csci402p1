/* library to provide a priority queue (heap implementation) with no 
 * dynamic memory
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#include "heap.h"

void swap_heap_elements(Message* start, int x, int y) {
    Message temp;
    memcopy((char*) &temp, (char*) start, sizeof(Message));

    /*temp.key = start[x].key;
    temp.senderId = start[x].senderId;
    temp.senderMbox = start[x].senderMbox;
    temp.pkt.senderId = sender[x].pkt.senderId;
    temp.pkt.senderId = sender[x].pkt.senderId;
    temp.pkt.senderId = sender[x].pkt.senderId;
    temp.pkt.senderId = sender[x].pkt.senderId;
    for(i=0; i < sizeof(Message); i++) {
        ((char*) &temp)[i] = ((char*) start)[i];
    }*/

    /*start[x] = start[y];*/
    /*start[x].key = start[y].key;
    start[x].senderId = start[y].senderId;
    start[x].senderMbox = start[y].senderMbox;
    start[x].pkt = sender[y].pkt;*/
    memcopy((char*) &start[x], (char*) &start[y], sizeof(Message));

    /*start[y] = temp;*/
    memcopy((char*) &start[y], (char*) &temp, sizeof(Message));
}

void Heap_Push(Message* start, int* length) {
    int x = *length;
    int xp = (x - 1) >> 1;
    (*length)++;

    /* compare new element with parent */
    while ((x > 0) && (start[x].key < start[xp].key)) {
        swap_heap_elements(start, x, xp);
        x = xp;
        xp = (x - 1) >> 1;
    }

    return;
}

Message Heap_ExtractMin(Message* start, int* length) {
    int x = 0;
    int xc1 = 1;
    int xc2 = 2;
    swap_heap_elements(start, 0, (*length) - 1);
    (*length)--;

    /* reheapify from the top */
    while (xc1 < *length) {
        /* if xc2 is off the end, bring it inside */
        if (xc2 >= *length) {
            xc2 = xc1;
        }
        /* find the lesser of two children */
        if (start[xc1].key > start[xc2].key) {
            xc1 = xc2;
        }

        /* swap if my lesser child is smaller */
        if (start[x].key <= start[xc1].key) {
            break;
        }
        swap_heap_elements(start, x, xc1);
        x = xc1;
        xc1 = (x << 1) + 1;
        xc2 = (x << 1) + 2;
    }

    return start[*length];
}

