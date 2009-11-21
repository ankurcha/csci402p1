/*  Provide a function to become the network thread
 *  that will work with our distributed hospital entities.
 *
 *  USC CS 402 Group 11
 *  Fall 2009
 *  Max Pflueger
 */

#include "p2pnetwork.h"
#include "syscall.h"
#include "print.h"

void network_thread(int mbox) {
    int senderId = 0;
    int senderMbox = 0;
    Packet myPacket;

    /* Begin an infinite loop where we wait for data from the network */
    while(true) {
        Packet_Receive(mbox, senderId, senderMbox, myPacket);

        /*TODO populate myPacket */

        if(myPacket.packetType != LOCAL_ENTITY) {
            /* process a packet from another entity on the network */
            processExternalPacket(myPacket);
        } else {
            /* process a packet from my matching entity thread */
            processLocalPacket(myPacket);
        }
    }
}

void processExternalPacket(Packet pkt) {
    /* Process this packet */
    switch(pkt.packetType) {
        case EMPTY:
            /*TODO update latest timestamp ? */
            break;

        case LOCK_CREATE:
        case LOCK_DESTROY:
            break;
        case LOCK_ACQUIRE:
            /*TODO check if I have or am waiting for this lock */
            break;
        case LOCK_RELEASE:
            /*TODO should  we make this LOCK_OK? */
            break;

        case CV_CREATE:
        case CV_DESTROY:
            break;
        case CV_WAIT:
            /*TODO add them to the queue */
            break;
        case CV_SIGNAL:
        case CV_BROADCAST:
            /*TODO these are the same right? */
            /*TODO I think I have to wake up the entity thread */
            break;

        case NODE_START:
        case NODE_STOP:
        case NODE_READY:
            /*TODO what do these do? */
            break;
        case default:
            break;
    }
}

void processLocalPacket(Packet pkt) {
}

