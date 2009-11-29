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
#include "heap.h"
#include "string.h"

#define MaxMsgQueue 200

void processExternalPacket(Packet pkt, int senderId, int senderMbox);
void processLocalPacket(Packet pkt);

void network_thread() {
    int i, j;
    int senderId = 0;
    int senderMbox = 0;
    Packet myPacket;
    Message msgQueue[MaxMsgQueue];
    Message* message;
    int queueLength = 0;
    unsigned int minTS;
    int machineIndex[7];
    int numEntities;
    int l = 0;
    unsigned int maxTS[MaxEntities];
    unsigned char buf[30];

    for (i = 0; i < MaxEntities; i++) {
        maxTS[i] = 0;
    }
    readyCount = 1;

    going = 0;
    /*myMbox = mbox;*/

    /* this array should make it easier to scan all entities */
    machineIndex[0] = 0;
    for (i = 1; i < 7; i++) {
        machineIndex[i] = machineIndex[i - 1] + numberOfEntities[i - 1];
    }
    numEntities = machineIndex[6] + numberOfEntities[6];
    if (numEntities > MaxEntities) {
        print("ERROR: numEntities > MaxEntities\n");
        Halt();
    }

    /* Begin an infinite loop where we wait for data from the network */
    while (1) {
        Packet_Receive(myMbox, &senderId, &senderMbox, &myPacket);
        print("Received a packet from senderId ");
        print(itoa(senderId, buf));
        print(" and mbox ");
        print(itoa(senderMbox, buf));
        print(" with timestamp ");
        print((char*) itoa(myPacket.timestamp, buf));
        print(" packetType: ");
        print(itoa(myPacket.packetType, buf));
        print("\n");
        if (senderMbox != 0) {
#ifdef DEBUG
            print("  From someone else!\n");
#endif
            /* process a packet from another entity on the network */
            /* updates the maxTS for this entitiy */
            /* senderId is the machine num, remember mbox 0 is reserved */
            i = machineIndex[senderId] + senderMbox - 1;
            if (myPacket.timestamp > maxTS[i]) {
                /*print("maxTS updated\n");*/
                maxTS[i] = myPacket.timestamp;
            } else {
                print("ASSUMPTION VIOLATED: packet received out of order\n");
                print("  from sender ");
                print((char*) itoa(senderId, buf));
                print(" and mbox ");
                print((char*) itoa(senderMbox, buf));
                print(".  New TS: ");
                print((char*) itoa(myPacket.timestamp, buf));
                print(" Old TS: ");
                print((char*) itoa(maxTS[i], buf));
                print("\n");
                Halt();
            }

            /*print("1");*/

            /* update minTS */
            minTS = maxTS[0];
            for (i = 1; i < numEntities; i++) {
                if (maxTS[i] < minTS) {
                    minTS = maxTS[i];
                }
            }

            /*print("2");*/

            /* some packet types do not need in-order processing, others do */
            switch(myPacket.packetType) {
                case EMPTY:
                case LOCK_ACQUIRE:
                case LOCK_RELEASE:
                case LOCK_OK:
                case NODE_READY:
                case DO_PING:
                case PING:
                case PONG:
                case KILL:
                case GO:

                    /*print("7");*/
                    processExternalPacket(myPacket, senderId, senderMbox);
                    /*print("8");*/
                    break;

                default:
                    /*print("3");*/
                    /* enqueue this packet */
                    if (queueLength < MaxMsgQueue) {
                        msgQueue[queueLength].senderId = senderId;
                        msgQueue[queueLength].senderMbox = senderMbox;
                        msgQueue[queueLength].pkt.senderId = myPacket.senderId;
                        msgQueue[queueLength].pkt.timestamp = myPacket.timestamp;
                        msgQueue[queueLength].pkt.packetType = myPacket.packetType;
                        strcpy(msgQueue[queueLength].pkt.data, myPacket.data);
                        msgQueue[queueLength].key = myPacket.timestamp;
                        Heap_Push(msgQueue, &queueLength);
                    } else {
                        print("ERROR: msgQueue ran out of space\n");
                        Halt();
                    }

                    /*print("4");*/

                    break;
            }

            /* process all messages up to minTS */
            while (msgQueue[0].key < minTS) {
                message = Heap_ExtractMin(msgQueue, &queueLength);

                /*print("5");*/

                myPacket.senderId = message->pkt.senderId;
                myPacket.timestamp = message->pkt.timestamp;
                myPacket.packetType = message->pkt.packetType;
                strcpy(myPacket.data, message->pkt.data);
                senderId = message->senderId;
                senderMbox = message->senderMbox;

                processExternalPacket(myPacket, senderId, senderMbox);

                /*print("6");*/
            }
        } else {
            /* process a packet from my matching entity thread */
            processLocalPacket(myPacket);
        }
    }
}

void processExternalPacket(Packet pkt, int senderId, int senderMbox) {
    int temp;
    int replies = -1;
    int numEntities = 0;
    int name;
    int i, j;
    Packet p;
    unsigned char buf[30];

    for (j = 0; j < 7; j++) {
        numEntities += numberOfEntities[j];
    }
    numEntities--;
    print("Processing External Packet Type: ");
    print(itoa(pkt.packetType,buf));
    print("\n");
    /* Process this packet */
    switch (pkt.packetType) {
        case EMPTY:
            print("Empty packet received!\n");
        case LOCK_ACQUIRE:
            print("Got a LOCK_ACQUIRE!!!\n");
            name = copyOutInt(pkt.data, NAME);
            /* check if I am holding this lock */
            print("Checking resource status\n");
            switch (getResourceStatus(name)) {
                case RES_HELD:
                    if (resources[name].timestamp > pkt.timestamp) {
                        print(
                                "ERROR: received an earlier request for a lock I already hold");
                        break;
                    }
                    /* fallthrough */
                case RES_REQ:
                    if (resources[name].timestamp < pkt.timestamp) {
                        /*TODO add them to the list */
                        MsgQueue_Push(&pendingLockQueue[name], &pkt, senderId, senderMbox); 
                        break;
                    }else if(resources[name].timestamp == pkt.timestamp){
                        if(senderId*1000 + senderMbox > GetMachineID() * 1000+myMbox){
                            MsgQueue_Push(&pendingLockQueue[name], &pkt, senderId, senderMbox);
                            break;
                        }
                    }
                    /* fallthrough */
                case RES_NONE:
                    /* Resource is not held, hence, we send out a LOCK_OK message
                     * construct a packet/
                     */
                    p.senderId = GetMachineID();
                    p.timestamp = GetTimestamp();
                    p.packetType = LOCK_OK;
                    copyInInt(p.data, NAME, name);
                    Packet_Send(senderId, senderMbox, GetMachineID(), &p);
                    break;
                default:
                    print("ERROR: invalid resource status\n");
            }
            break;
        case LOCK_OK:
            /* Got a LOCK_OK so now we need to check whether we requested this
             * Lock and if yes, we keep processing till all have replied with
             * LOCK_OK
             */
            /* get the lock being referred to */
            print("Received LOCK_OK from senderId: ");
            print(itoa(pkt.senderId, buf));
            print("\n");
            name = copyOutInt(pkt.data, NAME);
            if (getResourceStatus(name) == RES_REQ) {
                /* Yes, lock was requested */
                /* Update the number of replies that we have received so far */
                replies = getResourceReplies(name);
                replies++;
                updateResourceReplies(name, replies);
                print("Received LOCK_OK num: ");
                print((char*) itoa(replies, buf));
                print("\n");
                if (replies == numEntities) {
                    /* Now we have seen all the LOCK_OKs that we need and hence
                     * we get the LOCK NOW and delete the resource from the
                     * requestedResource and add it to the HeldResources
                     */
                    resources[name].state = RES_HELD;
                    /* Now we can send a signal to the entity */
                    Acquire(netthread_Lock);
                    Signal(netthread_CV, netthread_Lock);
                    Release(netthread_Lock);
                }
            }
            break;
        case CV_WAIT:
            /* add them to the queue of requests */
            print("Got a CV_WAIT!\n");
            name = copyOutInt(pkt.data, NAME); /* CVID */
            temp = copyOutInt(pkt.data, 4); /* LockID */
            MsgQueue_Push(&pendingCVQueue[name], &pkt, senderId, senderMbox);
            break;
        case CV_SIGNAL:
            print("Got CV_SIGNAL\n");
            Process_CV_Signal(pkt);
            break;
        case CV_BROADCAST:
            print("ERROR: external CV_BROADCAST packet received\n");
            Halt();
            break;
        case NODE_READY:
            /* add to the node ready count, once all nodes are ready,
             * start the simulation
             */
            print("Received NODE_READY\n");
            readyCount++;
            if (readyCount < numEntities) {
                break;
            }
            SendAll(GO);
            /* fallthrough */
        case GO:
            /* wake entity thread*/
            if(!going) {
                print("GOGOGO\n");
                Acquire(netthread_Lock);
                Signal(netthread_CV, netthread_Lock);
                Release(netthread_Lock);
                going = 1;
            }
            break;
        case RECP_DATA_UPDATE:
            temp = UpdateData_Receptionist(pkt);
            break;
        case PAT_DATA_UPDATE:
            temp = UpdateData_Patient(pkt);
            break;
        case DOORB_DATA_UPDATE:
            temp = UpdateData_Doorboy(pkt);
            break;
        case DOC_DATA_UPDATE:
            temp = UpdateData_Doctor(pkt);
            break;
        case CASH_DATA_UPDATE:
            temp = UpdateData_Cashier(pkt);
            break;
        case CLERK_DATA_UPDATE:
            temp = UpdateData_Clerk(pkt);
            break;
        case MAN_DATA_UPDATE:
            temp = UpdateData_HospitalManager(pkt);
            break;
        case GLOBAL_DATA_UPDATE:
            UpdateData_Global(pkt);
            break;
        case DO_PING:
            SendAll(PONG);
            break;
        case PING:
            print("Received PING\n");
            p.senderId = GetMachineID();
            p.timestamp = GetTimestamp();
            p.packetType = PONG;
            Packet_Send(senderId, senderMbox, myMbox, &p);
            break;
        case PONG:
            print("Received PONG\n");
            break;
        case KILL:
            Halt();
            break;
        default:
            break;
    }
}

void processLocalPacket(Packet pkt) {
    /* My entity thread wants my attention */
    /* Process this packet */
    int status = -1;
    int numEntities = 0;
    int temp, temp1;
    int i, j, k;
    int name;
    unsigned char str[20];
    Packet p;
    int senderId, senderMbox;
    for (j = 0; j < 7; j++) {
        numEntities += numberOfEntities[j];
    }
    numEntities--;

#ifdef DEBUG
    print("Processing Local Packet of type: ");
    print((char*) itoa(pkt.packetType, str));
    print("\n");
#endif
    switch (pkt.packetType) {
        case EMPTY:
            print("!!!!!!!!!!!!!!!!!!!!!EMPTY PACKET!!!!!!!!!!!!!!!!!\n");
            break;
        case LOCK_ACQUIRE:
            print("LOCAL_LOCK_ACQUIRE\n");
            name = copyOutInt(pkt.data, NAME);
            DistLock_Acquire(name);
            break;

        case LOCK_RELEASE:
            print("LOCK_RELEASE_LOCAL\n");
            name = copyOutInt(pkt.data, NAME);
            DistLock_Release(name);
            /* pop the request from the queue */

            /* no futher action */
            break;

        case CV_WAIT:
            name = copyOutInt(pkt.data, NAME); /* CVID */
            temp1 = copyOutInt(pkt.data, 4); /* LockID */
            DistCV_Wait(name, temp1);
            break;

        case CV_SIGNAL:
            name = copyOutInt(pkt.data, NAME); /* CVID */
            DistCV_Signal(name);
            /* When we want to send a signal, we should know who exactly to send
             * the signal to ie */
            break;
        case CV_BROADCAST:
            name = copyOutInt(pkt.data, NAME); /* CVID */
            while (!MsgQueue_IsEmpty(&pendingCVQueue[name])) {
                DistCV_Signal(name);
            }
            break;
        case NODE_READY:
#ifdef DEBUG
            print("SendAll NODE_READY\n");
#endif
            SendAll(NODE_READY);
            break;
        case RECP_DATA_UPDATE:
        case PAT_DATA_UPDATE:
        case DOORB_DATA_UPDATE:
        case DOC_DATA_UPDATE:
        case CASH_DATA_UPDATE:
        case CLERK_DATA_UPDATE:
        case MAN_DATA_UPDATE:
        case GLOBAL_DATA_UPDATE:
            /* Take care of sending updates */
            temp = DistUpdate_Send(pkt);
            break;
        case DO_PING:
            SendAll(DO_PING);
            break;
        case PING:
            Acquire(ping_Lock);
            Signal(ping_CV, ping_Lock);
            Release(ping_Lock);
            break;
        case PONG:
            print("ERROR: Entity send me a pong\n");
            break;
        case KILL:
            Kill();
            break;
        default:
            break;
    }
}

