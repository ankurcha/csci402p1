/* library to provide a priority queue (heap implementation) with no 
 * dynamic memory
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef HEAP_H
#define HEAP_H 1

struct heap_element {
    int key;
    void* value;
};
typedef struct heap_element heap_element;

/* methods for a priority queue */

/* push the element one past the end of the heap into the heap */
void Heap_Push(heap_element* start, int& length);

/* remove the minimum value from the heap */
heap_element Heap_ExtractMin(heap_element* start, int& length);

#endif /* HEAP_H */


