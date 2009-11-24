/*
 *  eventDispatacher.c
 *  
 *
 *  Created by Ankur Chauhan on 11/19/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */
#include "eventDispatcher.h"

void Init_MsgQueue(MessageQueue *q, QueueElement *queueElements, int length) {
    int i = 0;
    q->queue = queueElements;
    q->length = length;
    q->head = -1;
    q-> tail = -1;
    while (i < q->length) {
        q->queue[i].next = -1;
        q->queue[i].valid = 0;
        i++;
    }
    return;
}

void MsgQueue_Push(MessageQueue *q, Packet *msg, int senderId, int senderMbox) {
    int i = 0;
    while (i < q->length) {
        if (!q->queue[i].valid)
            break;
        i++;
    }
    if (i == q->length) {
        return;
    }
    if (q->tail == -1)
        q->head = i;
    else
        q->queue[q->tail].next = i;

    q->queue[i].valid = 1;
    q->queue[i].next = -1;
    q->queue[i].message = *msg;
    q->queue[i].senderId = senderId;
    q->queue[i].senderMbox = senderMbox;
    q->tail = i;
    return;
}

void MsgQueue_Pop(Packet *p, MessageQueue *q, int *senderId, int *senderMbox) {
    int temp;
    /* check for empty queue */
    if (q->head == -1) {
        return;
    }

    temp = q->head;

    /* check if we popped the last element */
    if (q->tail == q->head) {
        q->head = -1;
        q->tail = -1;
    } else {
        q->head = q->queue[q->head].next;
        if (q->tail == -1) {
            print("Error: queue malformed\n");
        }
    }
    q->queue[temp].valid = 0;
    q->queue[temp].next = -1;
    *senderId = q->queue[temp].senderId;
    *senderMbox = q->queue[temp].senderMbox;
    (*p).senderId = q->queue[temp].message.senderId;
    (*p).timestamp = q->queue[temp].message.timestamp;
    (*p).packetType = q->queue[temp].message.packetType;
    strcpy((*p).data, q->queue[temp].message.data);
}

char MsgQueue_IsEmpty(MessageQueue *q) {
    if (q->head == -1) {
        if (q->tail != -1)
            print("ERROR: queue is headless! run for your lives!\n");
        return 1;
    } else
        return 0;
}

void updateLastTimestampSeen(int hostID, int timestampReceived) {
    LastTimestampSeen[hostID] = timestampReceived;
}

