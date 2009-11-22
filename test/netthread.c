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

void processExternalPacket(Packet pkt, int senderId, int senderMbox);
void processLocalPacket(Packet pkt);

void network_thread(int mbox) {
    int senderId = 0;
    int senderMbox = 0;
    Packet myPacket;

    /* Begin an infinite loop where we wait for data from the network */
    while(true) {
        Packet_Receive(mbox, senderId, senderMbox, myPacket);

        /*TODO: populate myPacket */

        if(senderMbox = 0) {
            /* process a packet from another entity on the network */
            processExternalPacket(myPacket, senderId, senderMbox);
        } else {
            /* process a packet from my matching entity thread */
            processLocalPacket(myPacket);
        }
    }
}

void processExternalPacket(Packet pkt, int senderId, int senderMbox) {
    int temp;
    int replies = -1;
    int totalEntities = 0;
    int name;
    Packet p;
    for(j=0;j<7;j++){
        totalEntities += numberOfEntities[j];
    }

    /* Process this packet */
    switch(pkt.packetType) {
        case EMPTY:
            /* TODO: update latest timestamp ? */
            break;

        case LOCK_ACQUIRE:
            name = copyOutInt(pkt.data, NAME);

            /* check if I am holding this lock */
            switch(getResourceStatus(name)) {
                case RES_HELD:
                    if(myTS > pkt.timestamp) {
                        print("ERROR: received and earlier request for a lock I already hold");
                        break;
                    }
                case RES_REQ:
                    if(myTS < pkt.timestamp) {
                        /*TODO add them to the list */
                        break;
                    }
                    /* fallthrough */

                case RES_NONE:
                    /* Resource is not held, hence, we send out a LOCK_OK message
                     * construct a packet 
                     */
                    p.senderId = GetMachineId();
                    p.timestamp = GetTimestamp();
                    p.packet_type = LOCK_OK;
                    copyInInt(p.data, NAME, name);
                    Packet_Send(senderId, senderMbox, GetMachineId(), p);
                    break;

                default:
                    print("ERROR: invalid resource status\n");
            }
            break;

        case LOCK_OK:
            /*TODO should  we make this LOCK_OK? */
            /* Got a LOCK_OK so now we need to check whether we requested this
               Lock and if yes, we keep processing till all have replied with LOCK_OK
             */
                /* get the lock being referred to */
                temp = copyOutInt(pkt.data,0);
                /* If the lock was requested,*/ 
                if(IsResourcePresent(resourcesRequested, LOCK, temp) == 1) {
                    /* Yes, lock was requested */
                    /* Update the number of replies that we have received so far */
                    replies = getRepliesSeen(resourcesRequested, LOCK, temp);
                    updateReplies(resourcesRequested, LOCK, temp, replies+1);
                    replies = getRepliesSeen(resourcesRequested, LOCK, temp);
                    if(replies == totalEntities) {
                        /* Now we have seen all the LOCK_OKs that we need and hence
                         * we get the LOCK NOW and delete the resource from the 
                         * requestedResource and add it to the HeldResources
                         */
                        Acquire(netthread_Lock);
                        deleteResource(resourcesRequested, LOCK, temp);
                        addResource(HeldResources, LOCK, temp, 0);
                        /* Now we can send a signal to the entity */
                        Signal(netthread_CV, netthread_Lock);
                        Release(netthread_Lock);
                    }
                }
            break;

        case CV_WAIT:
            /* TODO: add them to the queue of requests */
                MsgQueue_Push(waitingNodes, pkt);
            break;
        case CV_SIGNAL:
        case CV_BROADCAST:
            /*TODO When we get a Signal we first will POP the waitingNodes Queue
             * Then check if the node associated with us is the one being signaled
             * If yes, we will wake it up
             */
            break;

        case NODE_READY:
            /*TODO: add to the node ready count, once all nodes are ready,
             * start the simulation
             */
            break;
        default:
            break;
    }
}

void processLocalPacket(Packet pkt) {
    /* My entity thread wants my attention */

    /* Process this packet */
    int status = -1;
    int totalEntities = 0;
    int temp, temp1;
    int name;
    Packet p;
    int i, senderId, senderMbox;
    for(j=0;j<7;j++){
        totalEntities += numberOfEntities[j];
    }

    switch(pkt.packetType){
        case LOCK_ACQUIRE:
            name = copyOutInt(pkt.data, NAME);
            DistLock_Acquire(name);
            /* now, once we receive enough OK's, 
             * we will wake the local entity */
            break;

        case LOCK_RELEASE:
            name = copyOutInt(pkt.data, NAME);
            DistLock_Release(name);
            /* no futher action */
            break;
            
        case CV_WAIT:
            /*TODO: add them to the queue */
            /* Get the lock ID and the CV ID */
            temp = copyOutInt(pkt.data, 0); /* CVID */
            temp1 = copyOutInt(pkt.data, 4); /* LockID */
            DistCV_Wait(temp, temp1);
            break;
        case CV_SIGNAL:
            /* When we want to send a signal, we should know who exactly to send
             * the signal to ie 
        case CV_BROADCAST:
            /*TODO: these are the same right? Yes they are*/
            /*TODO: I think I have to wake up the entity thread Yes*/
            break;
            
        case NODE_READY:
            /*TODO: what do these do? */
            break;
        case default:
            break;
    }
}

