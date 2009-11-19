/*
 *  eventDispatacher.c
 *  
 *
 *  Created by Ankur Chauhan on 11/19/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */
#include "eventDispatacher.h"


void Init_MsgQueue(MessageQueue *q){
    int i = 0;
    q->head = -1;
    q-tail = -1;
    while(i<q->length){
        q->queue[i].next = -1;
        q->queue[i].valid = 0;
        i++;
    }
    return;
}

int MsgQueue_Push(MessageQueue *q, Packet& msg){
    int i=0;
    while(i < q->length){
        if(!q->queue[i].valid)
            break;
        i++;
    }
    if(i == q->length){
        return = -1;
    }
    if(q->tail == -1)
        q->head = i;
    else
        q->queue[q->tail].next = i;
    
    q->queue[i].valid = 1;
    q->queue[i].next = -1;
    q->queue[i].message = msg;
    
    q->tail = i;
    return i;
}

Packet MsgQueue_Pop(MessageQueue *q){
    int temp;
    
    /* check for empty queue */
    if(q->head == -1){
        return NULL;
    }
    
    temp = q->head;
    
    /* check if we popped the last element */
    if(q->tail == q->head){
        q->head = -1;
        q->tail = -1;
    }else{
        q->head = q->queue[q->head].next;
        if(q->tail == -1){
            print("Error: queue malformed\n");
        }
    }
    q->queue[temp].valid = 0;
    q->queue[temp].next = -1;
    
    return q->queue[temp].message;
}


char MsgQueue_IsEmpty(MessageQueue *q){
    if(q->head == -1){
        if(q->tail != -1)
            print("ERROR: queue is headless! run for your lives!\n");
        return 1;
    }else
        return 0;
}

/* Method for the event Dispatcher this does all the sending */

/*Initialize the sendQueue at startup, all other methods assume this is done */

int eventDispatch(MessageQueue *q){
    int i = 0;
    int status = -1;
    Packet p;
    
    if(q == NULL)
        return -1;
    
    /* check if there is anything to send in the first place */
    if(MsgQueue_IsEmpty(q)){
        /* Queue is empty */
        return 0;
    }
    
    /* Now we just have go over the queue till there is nothing left */
    
    while(!MsgQueue_IsEmpty(q)){
        /* pop message of the queue */
        buf = MsgQueue_Pop(q);
        /* send this message to all the nodes */
        status = Hospital_Multicast( getHostList(), getMailboxList(), GetNumberOfHosts(), p);
        if(status != -1)
            i++;
        /* message was successfully send */
    }
    
    /* At this point the messages in the event queue for this time has been emptied */
    return i; /* we return the number of messages that were successfully sent */
}

/* This is used to add messages to the sendQueue for dispatch in the next cycle */
int ScheduleForSend(Packet p){
    int status = -1;
    status = MsgQueue_Push(sendQueue, p);
    return status;
}