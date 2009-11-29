#include "p2pnetwork.h"

/******************************
 ****** SYSTEM FUNCTIONS ******
 ******************************/

int getMboxNum() {
    int senderId;
    int senderMbox;
    unsigned char str[20];
    unsigned char message[MaxMailSize];
    myMbox = -1;
    Receive(0, &senderId, &senderMbox, message);
    myMbox = copyOutInt(message, 0);
    return myMbox;
}

void initializeSystem() {
    /* This function takes care of initializing the various
     * locks and other things at the system startup
     */
    /* Read all the configuration data */
    int i;
    print("Initializing Netthread\n");
    initResources();
    netthread_Lock = CreateLock("netthread_Lock");
    netthread_CV = CreateCondition("netthread_CV");
    ping_Lock = CreateLock("ping_Lock");
    ping_CV = CreateCondition("ping_CV");
    readConfig();
    getMboxNum();
    /* create system locks and CV*/

    for(i = 0;i<MAX_CV;i++)
        Init_MsgQueue(&pendingCVQueue[i], queue[i], MAX_CV_QUEUE_LEN);

    for(i = 0; i< MAX_LOCK;i++)
        Init_MsgQueue(&pendingLockQueue[i], lockqueue[i], MAX_LOCK_QUEUE_LEN);
}

void HMultiPing() {
    Packet p;

    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = DO_PING;
    Packet_Send(GetMachineID(), myMbox, 0, &p);
}

void SendAll(int packetType) {
    int i, j;
    Packet p;
    char buf[20];

    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = packetType;
#ifdef DEBUG
    print("Created a packet with timestamp ");
    print((char*) itoa(p.timestamp, buf));
    print("\n");
#endif
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            if (j == GetMachineID() && (i + 1) == myMbox) {
                continue;
            }
#ifdef DEBUG
            print("sendall-ing packet to machine ");
            print((char*)itoa(j, buf));
            print(" and mbox ");
            print((char*)itoa(i+1, buf));
            print(" from mbox ");
            print((char*)itoa(myMbox, buf));
            print("\n");
#endif
            Packet_Send(j, i + 1, myMbox, &p);
        }
    }
}

void HKill() {
    Packet p;

    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = KILL;

    Packet_Send(GetMachineID(), myMbox, myMbox, &p);
}

void Kill() {
    SendAll(KILL);
    Halt();
}

/***********************************
 ******* Resource Functions ********
 ***********************************/

int getResourceStatus(int name) {
    int status = RES_NONE;
    int i = 0;
    char str[20];
    if(name >= MAX_RESOURCES)
        return -1;
    else
        status =  resources[name].state;

    print("Resource status: ");
    print(itoa(status, str));
    print("\n");
    return status;
}

int getResourceTimestamp(int name) {
    if(name >= MAX_RESOURCES) {
        return -1;
    }
    if(resources[name].valid == 0) {
        return -1;
    }
    return resources[name].timestamp;
}

int updateResourceStatus(int name, int newStatus) {
    if(name >= MAX_RESOURCES)
        return -1;
    else
        return resources[name].state = newStatus;
}

int getResourceReplies(int name) {
    if(resources[name].valid == 0)
        return -1;
    else
        return resources[name].replies;
}

int updateResourceReplies(int name, int newReplies) {
    if(resources[name].valid == 0)
        return -1;
    else
        return resources[name].replies = newReplies;
}

void initResources() {
    int i = 0;
    for (i = 0; i < MAX_RESOURCES; i++)
        resources[i].valid = 0;
        resources[i].state = RES_NONE;
}

int addResource(int name, int state, int timestamp) {
    int i = 0;
    int targetPos = -1;
    char str[20];

    print("Adding Resource: ");
    print((char*) itoa(name, str));
    print(" RES_REQ\n");

    targetPos = name;
    if(targetPos > MAX_RESOURCES)
        return -1;
    resources[targetPos].name = name;
    resources[targetPos].timestamp = timestamp;
    resources[targetPos].valid = 1;
    resources[targetPos].replies = 0;
    resources[targetPos].state = state;
    return targetPos;
}

int deleteResource(int name) {
    int i = 0;
    if(name >= MAX_RESOURCES)
        return -1;
    else
        return resources[name].valid = 0;
}

int IsResourcePresent(int name) {
    int i = 0;
    if (resources[name].name == name && resources[name].valid == 1)
        return resources[name].valid;
    else
        return 0;
}

/*******************************
 ******* Lock Functions ********
 *******************************/
int HLock_Release(int HlockId) {
    int status = -1;
    Packet p;
    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = LOCK_RELEASE;
    copyInInt(p.data, 0, HlockId); /* Data part just contains the LockID */
    /* Check if I already own this lock */
    if (HlockId < 0)
        return status;
    /* Create message for Lock Release */
    /* Send message to announce release of the lock to the network entity */

    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    
    /* Check for successful Multicast */
    if (status > -1)
        status = 0;
    return status;
    /* Return success */
}

int HLock_Acquire(int HlockId) {
    int status = -1;
    /* For this operation we need to do a distributed concensus and then decide
     * who should get the lock, else we just keep waiting till we do get
     */
    Packet p;
    if(netthread_Lock == -1)
        print("netthread_lock = -1\n");
    
    if(netthread_CV == -1)
        print("netthread_CV = -1\n");

    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = LOCK_ACQUIRE;
    copyInInt(p.data, 0, HlockId); /* Data part just contains the LockID */

    /* We have now built the packet and now we should do the following
     * 1. Send a message to the network-thread(mbox)
     * receiver
     */
    Acquire(netthread_Lock);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    /* Now we have to wait for the for the netthread to reply to us with
     * a go ahead this is done using a CV and a lock.
     */
    Wait(netthread_CV, netthread_Lock);
    /* When we return we are sure that we have Acquired the lock */
    Release(netthread_Lock);
    return status;
}

int DistLock_Acquire(int name) {
    /* Send a lock acquire message to all the targets */
    /* Add the requested resource to the requestedResource Array */
    int i, j;
    int status;
    Packet p,pkt;
    int senderId, senderMBox;

    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = LOCK_ACQUIRE;
    copyInInt(p.data, 0, name); /* Data part just contains the LockID */
    status = addResource(name, RES_REQ, p.timestamp);
    if( status == -1){
        print("Problem Adding the RES_REQ\n");
        Halt();
    }
#ifdef DEBUG
    print("Requested Resource\n");
    print("Sending requests to all\n");
#endif
    print("Sending Acquire request to all\n");

    for (j = 0; j < 7; j++){ 
        for (i = 0; i < numberOfEntities[j]; i++){ 
            if (j == GetMachineID() && (i + 1) == myMbox){ 
                continue;
            }else{
                Packet_Send(j, i + 1, myMbox, &p);
            }
        }
    }
    return 0;
}

int DistLock_Release(int name) {
    int i, j, status;
    Packet p, pkt;
    int senderToId, senderToMbox;
    status = -1;
    while((status = MsgQueue_Pop(&p, &pendingLockQueue[name], &senderToId, &senderToMbox))!=-1){
        p.senderId = GetMachineID();
        p.timestamp = GetTimestamp();
        p.packetType = LOCK_OK;
        copyInInt(p.data, 0, name); /* Data part just contains the LockID */
        if (updateResourceStatus(name, RES_NONE) == 1) {
                    Packet_Send(senderToId, senderToMbox, myMbox, &p);
        }
    }

    return 1; /* successfully released lock */
}

/* this is called by an entity thread to signal a CV */
int HCV_Signal(int HCVId, int HLockId) {
    /* Get the receiverId from the queue */
    int status = -1;
    int CV_Lock = -1;
    Packet p;

    status = getResourceStatus(HLockId);
    if (status == RES_HELD) {
        CV_Lock = getCV_Lock_Mapping(HCVId);
        p.senderId = GetMachineID();
        p.timestamp = GetTimestamp();
        p.packetType = CV_SIGNAL;
        copyInInt(p.data, 0, HCVId);
        copyInInt(p.data, 4, HLockId);
        /* It is assumed that by now LockId is Held and CVID which is passed is
         * to be held */
        HLock_Acquire(CV_Lock);
        status = Packet_Send(GetMachineID(), myMbox, 0, &p);
        HLock_Release(CV_Lock);
        return 1;
        /* We reach this point only when we got a signal message for this CV */
    } else if (status == RES_REQ) {
        /* We were supposed to have had the lock acquired -- this is an error */
        print("ERROR: tried to signal a lock that was only requested\n");
        Halt();
        return 0;
    } else if (status == RES_NONE) {
        /* No record of any such Lock in my resource List!! */
        print("ERROR: tried to signal a lock we did not own\n");
        Halt();
        return -1;
    }
    return -2;
}

int HCV_Broadcast(int HCVId, int HLockId) {
    /* Get the receiverId from the queue */
    int status = -1;
    int CV_Lock = -1;
    Packet p;

    status = getResourceStatus(HLockId);
    if (status == RES_HELD) {
        CV_Lock = getCV_Lock_Mapping(HCVId);
        p.senderId = GetMachineID();
        p.timestamp = GetTimestamp();
        p.packetType = CV_BROADCAST;
        copyInInt(p.data, 0, HCVId);
        copyInInt(p.data, 4, HLockId);
        /* It is assumed that by now LockId is Held and CVID which is passed is
         * to be held */
        HLock_Acquire(CV_Lock);
        status = Packet_Send(GetMachineID(), myMbox, 0, &p);
        HLock_Release(CV_Lock);
        return 1;
        /* We reach this point only when we got a signal message for this CV */
    } else if (status == RES_REQ) {
        /* We were supposed to have had the lock acquired -- this is an error */
        print("ERROR: tried to signal a lock that was only requested\n");
        Halt();
        return 0;
    } else if (status == RES_NONE) {
        /* No record of any such Lock in my resource List!! */
        print("ERROR: tried to signal a lock we did not own\n");
        Halt();
        return -1;
    }
    return -2;
}

/* this is called by an entity thread to wait on a CV */
int HCV_Wait(int HCVId, int HLockId) {
    int status = -1;
    int CV_Lock = -1;
    Packet p;
    unsigned char buf[30];

    status = getResourceStatus(HLockId);

    print("Tyring to wait on CV ");
    print((char*) itoa(HCVId, buf));
    print(" with lock ");
    print((char*) itoa(HLockId, buf));
    print("\n");

    if (status == RES_HELD) {
        CV_Lock = getCV_Lock_Mapping(HCVId);

        p.senderId = GetMachineID();
        p.timestamp = GetTimestamp();
        p.packetType = CV_WAIT;
        copyInInt(p.data, 0, HCVId);
        copyInInt(p.data, 4, HLockId);

        /* It is assumed that by now LockId is Held and CVID which is passed is
         * to be held */
        Acquire(netthread_Lock);

        HLock_Acquire(CV_Lock);
        status = Packet_Send(GetMachineID(), myMbox, 0, &p);
        HLock_Release(CV_Lock);

        HLock_Release(HLockId);
        /* wait for the network thread to tell us to wake */
        Wait(netthread_CV, netthread_Lock);

        HLock_Acquire(HLockId);
        Release(netthread_Lock);
        return 1;
        /* We reach this point only when we got a signal message for this CV */
    } else if (status == RES_REQ) {
        /* We were supposed to have had the lock acquired -- this is an error */
        print("ERROR: tried to wait on a lock that was only requested\n");
        Halt();
        return 0;
    } else if (status == RES_NONE) {
        /* No record of any such Lock in my resource List!! */
        print("ERROR: tried to wait on a lock we did not own\n");
        Halt();
        return -1;
    }
    return -2;
}

int DistCV_Wait(int CVID, int LockID) {
    Packet p;
    int senderMBox = 0;
    int i, j;
    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = CV_WAIT;
    copyInInt(p.data, 0, CVID);
    copyInInt(p.data, 4, LockID);

    /* send to every entity */
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            if (j == GetMachineID() && (i + 1) == myMbox){
               continue;
            }else{
                Packet_Send(j, i + 1, myMbox, &p);
            }
        }
    }

    /* Also, we need to maintain a list waiting nodes */
    /* This will be popped when we receive a SIGNAL */
    MsgQueue_Push(&pendingCVQueue[CVID], &p, GetMachineID(), myMbox);
    return 1;
}

int DistCV_Signal(int CVID) {
    /* send this message to everyone so they all Pop from the 
     * pendingCVQueue[CVID] */
    int senderId;
    int senderMBox;
    int i, j;
    Packet p;
    Packet pkt;
    MsgQueue_Pop(&pkt, &pendingCVQueue[CVID], &senderId, &senderMBox);

    /* Process the Wait message */
    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = CV_SIGNAL;
    copyInInt(p.data, 0, copyOutInt(pkt.data, NAME)); /* CVID */
    copyInInt(p.data, 4, copyOutInt(pkt.data, 4)); /* Lock ID */

    /* send to every entity */
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            if (j == GetMachineID() && (i + 1) == myMbox){
               continue;
            }else{
                Packet_Send(j, i + 1, myMbox, &p);
            }
        }
    }

    if (senderId == GetMachineID() && senderMBox == myMbox) {
        print("ERROR: I signaled my own CV?\n");
        Halt();
    }

    return 1;
}

void Process_CV_Signal(Packet pkt) {
    /* When we get a Signal we first will POP the pendingCVQueue Queue
     * Then check if the node associated with us is the one being signaled
     * If yes, we will wake it up
     */
    int name;
    Packet p;
    int senderId, senderMbox;

    name = copyOutInt(pkt.data, NAME);
    MsgQueue_Pop(&p, &pendingCVQueue[name], &senderId, &senderMbox);

    if (senderId == GetMachineID() && senderMbox == myMbox) {
        /* wake up my entity */
        Acquire(netthread_Lock);
        Signal(netthread_CV, netthread_Lock);
        Release(netthread_Lock);
    }

}

void readConfig() {
    /* Read the configuration file given as the argument and parse the numbers
     * to the global variables
     */
    OpenFileId fd;
    unsigned char buf[10];
    unsigned char *temp = "";
    int num = 0;
    char number[10];
    int bytesread = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int start = 0;
    int end = -1;
    fd = Open("configfile", 10);
    print("Reading configfile...\n");
    while (i < 7) {
        bytesread = Read(buf, 1, fd);
        if (buf[0] != '\n') {
            temp[j] = buf[0];
            j++;
        } else {
            temp[j] = 0;
            num = atoi(temp);
            numberOfEntities[i] = num;
            j = 0;
            i++;
            temp = "";
        }
    }
    Close(fd);
    return;
}

int getCV_Lock_Mapping(int CVID) {
    int LockId = -1;
    LockId = CVID + LOCK_OFFSET;
    return LockId;
}
int DistUpdate_Send(Packet p) {
    /* This function is used to broadcast an Update packet to every one
     */
    int senderMBox = 0;
    int i, j;
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            if (j == GetMachineID() && (i + 1) == myMbox)
                continue;
            else 
                Packet_Send(j, i + 1, myMbox, &p);
        }
    }
}

/* Hospital Entity interface for the Network Data Update */
int HDataUpdate_Recp(int id, int peopleInLine, int currentToken) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Receptionist(&p, id, peopleInLine, currentToken);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}
int HDataUpdate_Pat(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    return status;
}
int HDataUpdate_Doorb(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    return status;
}
int HDataUpdate_Doc(int id, int peopleInLine, int prescription,
        int patientToken) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Doctor(&p, id, peopleInLine, prescription, patientToken);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}
int HDataUpdate_Cash(int id, int lineLength, int patToken, int fee,
        int payment, int sales) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Cashier(&p, id, lineLength, patToken, fee, payment, sales);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;

}
int HDataUpdate_Clerk(int id, int patientsInLine, int payment, int fee,
        int patPrescription, int sales) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Clerk(&p, id, patientsInLine, payment, fee, patPrescription,
            sales);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;

}
int HDataUpdate_HospMan(int id) {
    int status = -1;
    return status;
}

int HGlobalDataUpdate(short Variable, int val) {
    int status = -1;
    Packet p;
    buildPacket_GlobalData(&p, Variable, val);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}

int HGlobalListAppendUpdate(int key, int val) {
    int status = -1;
    Packet p;
    buildPacket_GlobalListAppend(&p, key, val);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}

int HGlobalQueuePopUpdate() {
    int status = -1;
    Packet p;
    buildPacket_GlobalQueuePop(&p);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}

int HGlobalQueuePushUpdate(int value) {
    int status = -1;
    Packet p;
    buildPacket_GlobalQueuePush(&p, value);
    status = Packet_Send(GetMachineID(), myMbox, 0, &p);
    return status;
}

/*
 * Get my mailbox number from GetMachineID():0
 */
int getMyNetThreadMbox() {
    int senderId, senderMbox;
    int mbox = 0;
    Packet p;
    int status = -1;
    int myMailboxNumber;
    /* Filled in by the initializing thread */
    status = Packet_Receive(mbox, &senderId, &senderMbox, &p);
    if (status != -1) {
        myMailboxNumber = copyOutInt(p.data, 0);
    }
    return myMailboxNumber;
}

/* Ping Network thread to see if it is alive */

void HPing_NetThread(){
    Packet p;
    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = PING;
    /* No Data */

    Acquire(ping_Lock);
    Packet_Send(GetMachineID(), myMbox, 0, &p);
    Wait(ping_CV, ping_Lock);
    Release(ping_Lock);
}

/* Send Packet declaring that the entity is ready to process */
void HNodeReady(){
    Packet p;
    p.senderId = GetMachineID();
    p.timestamp = GetTimestamp();
    p.packetType = NODE_READY;
    /* NO DATA */

    Acquire(netthread_Lock);
    Packet_Send(GetMachineID(), myMbox, 0, &p);
    Wait(netthread_CV, netthread_Lock);
    Release(netthread_Lock);
}
