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

int MsgQueue_SortedInsert(MessageQueue *q, Packet& msg){
    /* head is start */
    /* tail is end */
    
    int i=0;
    int target = 0;
    if (q->head == -1 && q->tail == -1) {
        /* first Element to be inserted */
        q->head = 0;
        q->tail = 0;
    }
    for (i=q->head; i<q->tail; i++) {
        if (q->queue[i].message.timestamp > msg.timestamp && q->queue[i].valid == 1) {
            /* find the position to insert at */
            target = i;
            break;
        }
    }
    /* found the position where i have to insert */
    /* make a hole */
    if (q->tail == q->length) {
        return -1;
    }
    for (i = q->tail-1; i>=target; i--) {
        q->queue[i+1] = q->queue[i];
    }
    /* insert msg at i */
    q->queue[target].message = msg;
    q->queue[target].valid = 1;
    q->tail = q->tail + 1;
}

Packet MsgQueue_Delete(MessageQueue *q, int LastTimestamp){
    /* This function returns smallest packet in the queue which has a timestamp
     * less or equal to than the LastTimestamp parameter
     */
    if(q == NULL || LastTimestamp<0)
        return NULL;
    int temp = q->head;
    if(q->tail == q->head){
        q->head = -1;
        q->tail = -1;
    }else if (q->queue[q->head].message.timestamp <= LastTimestamp) {
        q->head = q->queue[q->head].next;
        
        if(q->tail == -1){
            print("Error: queue malformed\n");
        }
        
        q->queue[temp].valid = 0;
        q->queue[temp].next = -1;
        
        return q->queue[temp].message;
    }

    return NULL;
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

int messageReceive(Packet &p){
    int i = 0;
    Packet *smallMsg;
    int smallest = 0;
    int smallestTimestamp = -1;
    if (p == NULL) {
        return -1;
    }
    /* Update the last timestamp seen table */
    updateLastTimestampSeen(p.senderId, p.timestamp);
    /* Queue the packet into the pending queue in timestamp order*/
    MsgQueue_SortedInsert(pendingMessagesQueue, p);
    /* Retrieve the earliest timestamp from my last timestamp received table
     * this value is the maximum timestamp for messages that I can process */
    for (i=0; i<MAXHOSTS; i++) {
        if(smallest > LastTimestampSeen[i]){
            smallest = i;
            smallestTimestamp = LastTimestampSeen[smallest];
        }
    }
    
    /* now we know the smallest timestamp among the packets seen so far 
     * hence we can now identify all the messages that we can successfully
     * process as the list is sorted.
     */
    smallMsg = MsgQueue_Delete(pendingMessagesQueue, smallestTimestamp);
    while (smallMsg != NULL) {
        /* Process the message */
        
        /* Get next message to process */
        smallMsg = MsgQueue_Delete(pendingMessagesQueue, smallestTimestamp);
    }
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

void updateLastTimestampSeen(int hostID, int timestampReceived){
    LastTimestampSeen[hostID] = timestampReceived;
}


