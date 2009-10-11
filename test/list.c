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

struct {
     

     ListElement *next;		/* next element on list,  */
				/* NULL if this is the last */
     int key;		    	/* priority, for a sorted list */
     void *item; 	    	/* pointer to item on the list */
}ListElement;

void __ListElement(ListElement *this, void *itemPtr, int sortKey);

/* The following class defines a "list" -- a singly linked list of */
/* list elements, each of which points to a single item on the list. */
/* */
/* By using the "Sorted" functions, the list can be kept in sorted */
/* in increasing order by "key" in ListElement. */

struct List {
     List();			/* initialize the list */
    ~List();			/* de-allocate the list */

    void Prepend(void *item); 	/* Put item at the beginning of the list */
    void Append(void *item); 	/* Put item at the end of the list */
    void *Remove(); 	 	/* Take item off the front of the list */

    void Mapcar(VoidFunctionPtr func);	/* Apply "func" to every element  */
					/* on the list */
    char IsEmpty();		/* is the list empty?  */
    

    /* Routines to put/get items on/off list in order (sorted by key) */
    void SortedInsert(void *item, int64_t sortKey);	/* Put item into list */
    void *SortedRemove(int64_t *keyPtr); 	  	/* Remove first item from list */
    ListElement *first;  	/* Head of the list, NULL if list is empty */
    ListElement *last;		/* Last element of list */
};


/*---------------------------------------------------------------------- */
/* ListElement::ListElement */
/*	Initialize a list element, so it can be added somewhere on a */list.
/* */
/*	"itemPtr" is the item to be put on the list.  It can be a pointer */
/*		to anything. */
/*	"sortKey" is the priority of the item, if any. */
/*---------------------------------------------------------------------- */

ListElement::ListElement(void *itemPtr, int64_t sortKey)
{
    item = itemPtr;
    key = sortKey;
    next = NULL;	/*assume we'll put it at the end of the list */
}

/*---------------------------------------------------------------------- */
/* List::List */
/*	Initialize a list, empty to start with. */
/*	Elements can now be added to the list. */
/*---------------------------------------------------------------------- */

List::List()
{ 
    first = last = NULL; 
}

/*---------------------------------------------------------------------- */
/* List::~List */
/*	Prepare a list for deallocation.  If the list still contains any  */
/*	ListElements, de-allocate them.  However, note that we do *not* */
/*	de-allocate the "items" on the list -- this module allocates */
/*	and de-allocates the ListElements to keep track of each item, */
/*	but a given item may be on multiple lists, so we can't */
/*	de-allocate them here. */
/*---------------------------------------------------------------------- */

List::~List()
{ 
    while (Remove() != NULL)
        ;	 /*delete all the list */elements
}

/*---------------------------------------------------------------------- */
/* List::Append */
/*     Append an "item" to the end of the */list.
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only */element.
/*	Otherwise, put it at the end. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*---------------------------------------------------------------------- */

void
List::Append(void *item)
{
    ListElement *element = new ListElement(item, 0);
    
    if (IsEmpty()) {		/*list is */empty
        first = element;
        last = element;
    } else {			/*else put it after */last
        last->next = element;
        last = element;
    }
}

/*---------------------------------------------------------------------- */
/* List::Prepend */
/*     Put an "item" on the front of the */list.
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only */element.
/*	Otherwise, put it at the beginning. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*---------------------------------------------------------------------- */

void
List::Prepend(void *item)
{
    ListElement *element = new ListElement(item, 0);
    
    if (IsEmpty()) {		/*list is */empty
        first = element;
        last = element;
    } else {			/*else put it before */first
        element->next = first;
        first = element;
    }
}

/*---------------------------------------------------------------------- */
/* List::Remove */
/*     Remove the first "item" from the front of the */list.
/*  */
/* Returns: */
/*	Pointer to removed item, NULL if nothing on the list. */
/*---------------------------------------------------------------------- */

void *
List::Remove()
{
    return SortedRemove(NULL);  /*Same as SortedRemove, but ignore the */key
}

/*---------------------------------------------------------------------- */
/* List::Mapcar */
/*	Apply a function to each item on the list, by walking through   */
/*	the list, one element at a time. */
/* */
/*	Unlike LISP, this mapcar does not return anything! */
/* */
/*	"func" is the procedure to apply to each element of the list. */
/*---------------------------------------------------------------------- */

void
List::Mapcar(VoidFunctionPtr func)
{
    for (ListElement *ptr = first; ptr != NULL; ptr = ptr->next) {
        DEBUG('l', "In mapcar, about to invoke %x(%x)\n", func, ptr->item);
        (*func)((int)ptr->item);
    }
}

/*---------------------------------------------------------------------- */
/* List::IsEmpty */
/*     Returns TRUE if the list is empty (has no */items).
/*---------------------------------------------------------------------- */

char
List::IsEmpty() 
{ 
    if (first == NULL)
        return TRUE;
    else
        return FALSE; 
}

/*---------------------------------------------------------------------- */
/* List::SortedInsert */
/*     Insert an "item" into a list, so that the list elements */are
/*	sorted in increasing order by "sortKey". */
/*     */
/*	Allocate a ListElement to keep track of the item. */
/*     If the list is empty, then this will be the only */element.
/*	Otherwise, walk through the list, one element at a time, */
/*	to find where the new item should be placed. */
/* */
/*	"item" is the thing to put on the list, it can be a pointer to  */
/*		anything. */
/*	"sortKey" is the priority of the item. */
/*---------------------------------------------------------------------- */

void
List::SortedInsert(void *item, int64_t sortKey)
{
    ListElement *element = new ListElement(item, sortKey);
    ListElement *ptr;		/*keep */track
    
    if (IsEmpty()) {	/*if list is empty, */put
        first = element;
        last = element;
    } else if (sortKey < first->key) {	
		/*item goes on front of */list
        element->next = first;
        first = element;
    } else {		/*look for first elt in list bigger than */item
        for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
            if (sortKey < ptr->next->key) {
                element->next = ptr->next;
                ptr->next = element;
                return;
            }
        }
        last->next = element;		/*item goes at end of */list
        last = element;
    }
}

/*---------------------------------------------------------------------- */
/* List::SortedRemove */
/*     Remove the first "item" from the front of a sorted */list.
/*  */
/* Returns: */
/*	Pointer to removed item, NULL if nothing on the list. */
/*	Sets *keyPtr to the priority value of the removed item */
/*	(this is needed by interrupt.cc, for instance). */
/* */
/*	"keyPtr" is a pointer to the location in which to store the  */
/*		priority of the removed item. */
/*---------------------------------------------------------------------- */

void *
List::SortedRemove(int64_t *keyPtr)
{
    ListElement *element = first;
    void *thing;
    
    if (IsEmpty()) 
        return NULL;
    
    thing = first->item;
    if (first == last) {	/*list had one item, now has none */
        first = NULL;
        last = NULL;
    } else {
        first = element->next;
    }
    if (keyPtr != NULL)
        *keyPtr = element->key;
    delete element;
    return thing;
}