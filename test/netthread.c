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

        /*TODO: populate myPacket */

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
    int temp;
    int replies = -1;
    int totalEntities = 0;
    for(j=0;j<7;j++){
        totalEntities += numberOfEntities[j];
    }
    /* Process this packet */
    switch(pkt.packetType) {
        case EMPTY:
            /* TODO: update latest timestamp ? */
            break;

        case LOCK_CREATE:
        case LOCK_DESTROY:
            break;
        case LOCK_ACQUIRE:
            /* TODO: check if I have or am waiting for this lock */
            /* Check if I am currently holding the packet */
            temp = copyOutInt(pkt.data,0);
            /* check if I am holding this lock */
            if (!IsResourceHeld(CV, temp) {
                /* Resource is not held, hence, we send out a LOCK_OK message
                 * construct a packet 
                 */
                Packet p;
                p.senderId = GetMachineId();
                p.timestamp = GetTimestamp();
                p.packet_type = LOCK_OK;
                copyInInt(p.data, 0, temp);
                status = Packet_Send(receiverID, recMBox, GetMachineId(), p);
            }else{
                /* I hold the lock, hence I am god!!! muhahahaha!!!*/
            }
            break;
        case LOCK_OK:
            /* We got a LOCK_OK so now we need to check whether we requested this
               Lock and if yes, we keep processing till all have replied with LOCK_OK
             */
                /* get the lock being referred to */
                temp = copyOutInt(pkt.data,0);
                /* If the lock was requested,*/ 
                if(IsResourcePresent(resourcesRequested, LOCK, temp) == 1){
                    /* Yes, lock was requested */
                    /* Update the number of replies that we have received so far */
                    replies = getRepliesSeen(resourcesRequested, LOCK, temp);
                    updateReplies(resourcesRequested, LOCK, temp, replies+1);
                    replies = getRepliesSeen(resourcesRequested, LOCK, temp);
                    if(replies == totalEntities){
                        /* Now we have seen all the LOCK_OKs that we need and hence
                         * we get the LOCK NOW
                         */
                        /* Delete the resource from the requestedResource and add
                         * it to the HeldResources
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

        case CV_CREATE:
        case CV_DESTROY:
            break;
        case CV_WAIT:
            /* TODO: add them to the queue of requests */
            break;
        case CV_SIGNAL:
        case CV_BROADCAST:
            /*TODO: these are the same right? Yes they are*/
            /*TODO: I think I have to wake up the entity thread Yes*/
            break;

        case NODE_START:
        case NODE_STOP:
        case NODE_READY:
            /*TODO: what do these do? */
            break;
        case default:
            break;
    }
}

void processLocalPacket(Packet pkt) {
    /* Process this packet */
    int status = -1;
    int totalEntities = 0;
    int temp, temp1;
    Packet p;
    int i, senderId, senderMbox;
    for(j=0;j<7;j++){
        totalEntities += numberOfEntities[j];
    }
    switch(packet_type){
        case EMPTY:
            /*TODO: update latest timestamp ? */
            break;
            
        case LOCK_CREATE:
        case LOCK_DESTROY:
            break;
        case LOCK_ACQUIRE:
            /*TODO: Send a lock acquire message to all the targets */
            /* Add the requested resource to the requestedResource Array */
            temp = copyOutInt(pkt.data,0);
            addResource(resourcesRequested, LOCK, temp, 0);
            for(j=0;j<7;j++){
                for(int i=0;i<numberOfEntities[j];i++){
                    /* TODO: Send to each entity. how? we need the receiverId and recMBox */
                    /* Sending LOCK_ACQUIRE to all and waiting for LOCK_OK */
                    Packet_Send(receiverId, recMBox, 0, pkt);
                }
            }
            break;
        case LOCK_OK:
            /* We just need to take the lock out of the heldResources List */
            temp = copyOutInt(pkt.data,0);
            if(deleteResource(HeldResources,LOCK, temp) == 1){
                /* Lock is no longer held by the lock
                 * Just broadcast this message to all the othe nodes */
                for(j=0;j<7;j++){
                    for(int i=0;i<numberOfEntities[j];i++){
                        /* TODO: Send to each entity how? we need the receiverId and recMBox */
                        Packet_Send(receiverId, recMBox, 0, pkt);
                    }
                }
            }
            break;
            
        case CV_CREATE:
        case CV_DESTROY:
            break;
        case CV_WAIT:
            /*TODO: add them to the queue */
            /* Get the lock ID and the CV ID */
            temp = copyOutInt(pkt.data, 0); /* CVID */
            temp1 = copyOutInt(pkt.data, 4); /* LockID */
            /* Check if I hold the lock and the CB */
            if (IsResourcePresent(HeldResources, CV, temp) && 
                    IsResourcePresent(HeldResources, LOCK, temp1)) {
                /* YES!! I am in the CR, now its ok to mess around */
                /* Release the conditionLock */
                if(deleteResource(HeldResources, LOCK, temp1) == 1){
                    /* After Releasing the lock we will move the CV to RequestedResources
                     * till we get a Signal */
                    addResource(resourcesRequested, CV, temp1, 0);
                    deleteResource(HeldResources, CV, temp);
                    /* Lock is no longer held by the lock
                     * Just broadcast this message to all the othe nodes 
                     * Build the packet to be sent around */
                    p.senderId = GetMachineID();
                    p.timestamp = GetTimestamp();
                    p.packetType = LOCK_OK;
                    copyInInt(p.data, 0, temp);
                    /* Now send p to all other nodes */
                    for(j=0;j<7;j++){
                        for(i=0;i<numberOfEntities[j];i++){
                            /* 
                             * TODO: Send to each entity how? we need 
                             * the receiverId and recMBox 
                             */
                            Packet_Send(receiverId, recMBox, 0, p);
                        }
                    }
                }

                /* At this point all the locks have been released */
                /* We can now send the CV_WAIT to all the nodes */
                for(j=0;j<7;j++){
                    for(i=0;i<numberOfEntities[j];i++){
                        Packet_Send(receiverId, recMBox, 0, pkt);
                    }
                }
                /* Wait until you receive a signal and it is for you*/
            }
            break;
        case CV_SIGNAL:
        case CV_BROADCAST:
            /*TODO: these are the same right? Yes they are*/
            /*TODO: I think I have to wake up the entity thread Yes*/
            break;
            
        case NODE_START:
        case NODE_STOP:
        case NODE_READY:
            /*TODO: what do these do? */
            break;
        case default:
            break;
    }
}

