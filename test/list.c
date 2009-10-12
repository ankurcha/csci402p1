/* list.h  */
/*	Data structures to manage LISP-like lists.   */
/* */
/*      As in LISP, a list can contain any type of data structure */
/*	as an item on the list: thread control blocks,  */
/*	pending interrupts, etc.  That is why each item is a "void *", */
/*	or in other words, a "pointers to anything". */
/* */
/* Copyright (c) 1992-1993 The Regents of the University of California. */
/* All rights reserved.  See copyright.h for copyright notice and limitation  */
/* of liability and disclaimer of warranty provisions. */

/* The following class defines a "list element" -- which is */
/* used to keep track of one item on a list.  It is equivalent to a */
/* LISP cell, with a "car" ("next") pointing to the next element on the list, */
/* and a "cdr" ("item") pointing to the item on the list. */
/* */
/* Internal data structures kept public so that List operations can */
/* access them directly. */


struct _ListElement{
    struct _ListElement *next;		/* next element on list,  */
                            /* 0 if this is the last */
    int key;		    	/* priority, for a sorted list */
    void *item; 	    	/* pointer to item on the list */
};
typedef struct _ListElement ListElement;

/* The following class defines a "list" -- a singly linked list of */
/* list elements, each of which points to a single item on the list. */
/* */
/* By using the "Sorted" functions, the list can be kept in sorted */
/* in increasing order by "key" in ListElement. */

 struct _List {
    ListElement *first;  	/* Head of the list, 0 if list is empty */
    ListElement *last;		/* Last element of list */
};
typedef struct _List List;

void List_SortedInsert(List *this, void *item, int sortKey);
char List_IsEmpty(List *this);
void * List_SortedRemove(List *this, int *keyPtr);

/*---------------------------------------------------------------------- */
/* __ListElement */
/*	Initialize a list element, so it can be added somewhere on a list.
/* */
/*	"itemPtr" is the item to be put on the list.  It can be a pointer */
/*		to anything. */
/*	"sortKey" is the priority of the item, if any. */
/*---------------------------------------------------------------------- */

void __ListElement(ListElement *this, void *itemPtr, int sortKey)
{
    this->item = itemPtr;
    this->key = sortKey;
    this->next = 0;	/*assume we'll put it at the end of the list */
}

/*---------------------------------------------------------------------- */
/* __List */
/*	Initialize a list, empty to start with. */
/*	Elements can now be added to the list. */
/*---------------------------------------------------------------------- */

void __List(List *this)
{ 
    this->first = this->last = 0; 
}

/*---------------------------------------------------------------------- */
/*  _List */
/*	Prepare a list for deallocation.  If the list still contains any  */
/*	ListElements, de-allocate them.  However, note that we do *not* */
/*	de-allocate the "items" on the list -- this module allocates */
/*	and de-allocates the ListElements to keep track of each item, */
/*	but a given item may be on multiple lists, so we can't */
/*	de-allocate them here. */
/*---------------------------------------------------------------------- */

void _List(List *this)
{ 
    while (Remove(this) != 0)
        ;	 /*delete all the list elements*/
}

/*---------------------------------------------------------------------- */
/* List_Append */
/*     Append an "item" to the end of the list.
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only element.
/*	Otherwise, put it at the end. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*---------------------------------------------------------------------- */

void List_Append(List *this, void *item)
{
    ListElement *element = (ListElement*) malloc(sizeof(ListElement));
    __ListElement(element, item, 0);
    
    if (List_IsEmpty(this)) {		/*list is empty */
        this->first = element;
        this->last = element;
    } else {			/*else put it after last */
        this->last->next = element;
        this->last = element;
    }
}

/*---------------------------------------------------------------------- */
/* List_Prepend */
/*     Put an "item" on the front of the list.
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only element.
/*	Otherwise, put it at the beginning. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*---------------------------------------------------------------------- */

void
List__Prepend(List *this, void *item)
{
    ListElement *element = (ListElement*) malloc(sizeof(ListElement));
    __ListElement(element, item, 0);
    
    if (IsEmpty()) {		/*list is empty*/
        this->first = element;
        this->last = element;
    } else {			/*else put it before first*/
        element->next = this->first;
        this->first = element;
    }
}

/*---------------------------------------------------------------------- */
/* List_Remove */
/*     Remove the first "item" from the front of the list.*/
/*  */
/* Returns: */
/*	Pointer to removed item, 0 if nothing on the list. */
/*---------------------------------------------------------------------- */

void *
List_Remove(List *this)
{
    return List_SortedRemove(this, 0);  /*Same as SortedRemove, but ignore the key*/
}

/*---------------------------------------------------------------------- */
/* List_Mapcar */
/*	Apply a function to each item on the list, by walking through   */
/*	the list, one element at a time. */
/* */
/*	Unlike LISP, this mapcar does not return anything! */
/* */
/*	"func" is the procedure to apply to each element of the list. */
/*---------------------------------------------------------------------- */

void List_Mapcar(List *this,int func)
{  
	ListElement *ptr;
	  
    for (ptr = this->first; ptr != 0; ptr = ptr->next) {
        ((int)ptr->item);
    }
}

/*---------------------------------------------------------------------- */
/* List_IsEmpty */
/*     Returns TRUE if the list is empty (has no items).
/*---------------------------------------------------------------------- */

char List_IsEmpty(List *this) 
{ 
    if (this->first == 0)
        return 1;
    else
        return 0; 
}

/*---------------------------------------------------------------------- */
/* List_SortedInsert */
/*     Insert an "item" into a list, so that the list elements are*/
/*	sorted in increasing order by "sortKey". */
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only element.*/
/*	Otherwise, walk through the list, one element at a time, */
/*	to find where the new item should be placed. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*	"sortKey" is the priority of the item. */
/*---------------------------------------------------------------------- */

void
List_SortedInsert(List *this, void *item, int sortKey)
{
    ListElement *element = (ListElement*) malloc(sizeof(ListElement));
    __ListElement(element, item, sortKey);
    
    
    
    if (List_IsEmpty(this)) {	/*if list is empty, put */
        this->first = element;
        this->last = element;
    } else if (sortKey < this->first->key) {	
		/*item goes on front of list */
        element->next = this->first;
        this->first = element;
    } else {		/*look for first elt in list bigger than item*/
    	ListElement *ptr;
        for (ptr =this->first; ptr->next != 0; ptr = ptr->next) {
            if (sortKey < ptr->next->key) {
                element->next = ptr->next;
                ptr->next = element;
                return;
            }
        }
        this->last->next = element;		/*item goes at end of list*/
        this->last = element;
    }
}

/*---------------------------------------------------------------------- */
/* List_SortedRemove */
/*     Remove the first "item" from the front of a sorted list.*/
/*  */
/* Returns: */
/*	Pointer to removed item, 0 if nothing on the list. */
/*	Sets *keyPtr to the priority value of the removed item */
/*	(this is needed by interrupt.cc, for instance). */
/* */
/*	"keyPtr" is a pointer to the location in which to store the  */
/*		priority of the removed item. */
/*---------------------------------------------------------------------- */

void * List_SortedRemove(List *this, int *keyPtr)
{
    ListElement *element = this->first;
    void *thing;
    
    if (List_IsEmpty(this)) 
        return 0;
    
    thing = this->first->item;
    if (this->first == this->last) {	/*list had one item, now has none */
        this->first = 0;
        this->last = 0;
    } else {
        this->first = element->next;
    }
    if (keyPtr != 0)
        *keyPtr = element->key;
    free (element);
    return thing;
}