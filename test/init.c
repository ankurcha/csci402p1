/*
 *  init.cpp
 *
 *  Hospital management simulation threads
 *  
 *
 *  Created by Ankur Chauhan on 9/15/09.
 *  USC CSCI 402 Operating Systems
 *  Group 11
 *    Ankur Chauhan, ankurcha
 *    Max Pflueger, pflueger
 *    Aneesha Mathew, aneesham
 *
 */

#include "syscall.h"
#include "itoa.c"
#include "print.c"

#define BUSY 0
#define FREE 1
#define SLEEPING 2

struct ListElement {
    ListElement(void *itemPtr, int64_t sortKey);	/* initialize a list element */
    ListElement *next;		/* next element on list,  */
    /* NULL if this is the last */
    int64_t key;		    	/* priority, for a sorted list */
    void *item; 	    	/* pointer to item on the list */
};

main(){
    
}