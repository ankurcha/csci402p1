#include "p2pnetwork.h"

/***********************************
 ******* Resource Functions ********
 ***********************************/

int getResourceStatus(char* name) {
    int status = RES_NONE;
    int i = 0;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (!strcmp(resources[i].name, name) && resources[i].valid == 1) {
            switch (resources[i].state) {
                case RES_HELD:
                    status = RES_HELD;
                    break;
                case RES_REQ:
                    status = RES_REQ;
                    break;
                default:
                    status = RES_NONE;
                    break;
            }
        }
    }
    return status;

}

int updateResourceStatus(char* name, int newStatus) {
    int i;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].valid == 1 && !strcmp(resources[i].name, name)) {
            resources[i].state = newStatus;
            return resources[i].state;
        }
    }
    return -1;
}

int getResourceReplies(int resourceID) {
    int i;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].valid == 1 && !strcmp(resources[i].name, name)) {
            return resources[i].replies;
        }
    }
    return -1;
}

int updateResourceReplies(char *name, int newReplies) {
    int i;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].valid == 1 && !strcmp(resources[i].name, name)) {
            resources[i].replies = newReplies;
            return resources[i].replies;
        }
    }
    return -1;
}

void initResources() {
    int i = 0;
    for (i = 0; i < MAX_RESOURCES; i++)
        resources[i].valid = 0;
}

int addResource(char *name, int state) {
    int i = 0;
    int targetpos = -1;
    for (i = 0; i < MAX_RESOURCES; i++)
        if (resources[i].valid = 0) {
            targetpos = i;
            break;
        }
    if (targetPos == -1)
        return -1;

    resources[targetPos].name = strcpy(resources[targetPos].name, name);
    resources[targetPos].timestamp = GetTimestamp();
    resources[targetPos].valid = 1;
    resources[targetPos].replies = 0;
    resources[targetPos].state = state;
    return targetPos;
}

int deleteResource(Resource arr[], char* name) {
    int i = 0;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (!strcmp(resources[i].name, name)) {
            resources[i].valid = 0;
            return 1;
        }
    }
    return 0;
}

int IsResourcePresent(Resource arr[], char* name) {
    int i = 0;
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (!strcmp(resources[i].name, name) && resources[i].valid == 1) {
            return 1;
        }
    }
    return 0;
}

/*******************************
 ******* Lock Functions ********
 *******************************/
int HLock_Release(int HlockId) {
    int status = -1;
    /* Check if I already own this lock */
    if (HlockId < 0)
        return status;
    /* Create message for Lock Release */
    /* Send message to announce release of the lock to the network entity */
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
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
    int status = -1;
    Packet p;
    p.senderId = GetMachineId();
    p.timestamp = GetTimestamp();
    p.packetType = LOCK_ACQUIRE;
    copyInInt(p.data, 0, HlockId); /* Data part just contains the LockID */

    /* We have now built the packet and now we should do the following
     * 1. Send a message to the network-thread(mbox)
     * receiver
     */
    Acquire(netthread_Lock);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
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
    addResource(resourcesRequested, LOCK, temp, 0);
    for (j = 0; j < 7; j++) {
        for (int i = 0; i < numberOfEntities[j]; i++) {
            /* Sending LOCK_ACQUIRE to all and waiting for LOCK_OK */
            Packet_Send(j, i + 1, myMbox, pkt);
        }
    }
}

int DistLock_Release(int name) {
    /* We just need to take the lock out of the heldResources List */
    temp = copyOutInt(pkt.data, 0);
    if (updateResourceStatus(temp, RES_NONE) == 1) {
        /* Lock is no longer held by the lock
         * Just broadcast this message to all the othe nodes */
        for (j = 0; j < 7; j++) {
            for (int i = 0; i < numberOfEntities[j]; i++) {
                Packet_Send(j, i + 1, myMbox, pkt);
            }
        }
        return 1; /* successfully released lock */
    }
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

        p.senderId = GetMachineId();
        p.timestamp = GetTimestamp();
        p.packetType = CV_SIGNAL;
        copyInInt(p.data, 0, HCVId);
        copyInInt(p.data, 4, HLockId);

        /* It is assumed that by now LockId is Held and CVID which is passed is
         * to be held */

        HLock_Acquire(CV_Lock);
        status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
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

    status = getResourceStatus(HLockId);
    if (status == RES_HELD) {
        CV_Lock = getCV_Lock_Mapping(HCVId);

        p.senderId = GetMachineId();
        p.timestamp = GetTimestamp();
        p.packetType = CV_WAIT;
        copyInInt(p.data, 0, HCVId);
        copyInInt(p.data, 4, HLockId);

        /* It is assumed that by now LockId is Held and CVID which is passed is
         * to be held */
        Acquire(netthread_Lock);

        HLock_Acquire(CV_Lock);
        status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
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

    p.senderId = GetMachineId();
    p.timestamp = GetTimestamp();
    p.packetType = CV_WAIT;
    copyInInt(p.data, 0, CVID);
    copyInInt(p.data, 4, LockID);

    /* send to every entity */
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            Packet_Send(j, i + 1, myMbox, p);
        }
    }

    /* Also, we need to maintain a list waiting nodes */
    /* This will be popped when we receive a SIGNAL */
    MsgQueue_Push(pendingCVQueue[CVID], p, GetMachineId(), myMbox);
    return 1;
}

int DistCV_Signal(int CVID) {
    /* send this message to everyone so they all Pop from the 
     * pendingCVQueue[CVID] */
    int senderId;
    int senderMBox;
    Packet p;
    Packet pkt = MsgQueue_Pop(pendingCVQueue[CVID], &senderId, &senderMBox);

    /* Process the Wait message */
    p.senderId = GetMachineId();
    p.timestamp = GetTimestamp();
    p.packetType = CV_SIGNAL;
    copyInInt(p.data, 0, copyOutInt(pkt.data, NAME)); /* CVID */
    copyInInt(p.data, 4, copyOutInt(pkt.data, 4)); /* Lock ID */

    /* send to every entity */
    for (j = 0; j < 7; j++) {
        for (i = 0; i < numberOfEntities[j]; i++) {
            Packet_Send(j, i + 1, myMbox, p);
        }
    }

    if (senderId == GetMachineId() && senderMBox == myMbox) {
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
    p = MsgQueue_Pop(pendingCVQueue[name], &senderId, &senderMbox);

    if (senderId == GetMachineId() && senderMBox == myMbox) {
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
    char buf[100];
    char temp[10];
    int num;
    int bytesread;
    int i = 0;
    int k;
    int start = 0;
    int end = -1;
    fd = Open("configfile", 10);
    bytesread = Read(buf, 100, fd);
    Close(fd);
    end = 0;
    for (k = 0; k < 7; k++) {
        /* Parse the comma separated numbers */
        for (i = 0; i < bytesread; i++) {
            if (buf[i] != ',') {
                end++;
            }
        }
        /* copy the number string to temp */
        for (j = start; j < end; j++)
            temp[j - start] = buf[j];
        print("Reading Config:");
        print(temp);
        print("\n");
        numberOfEntities[k] = atoi(temp);
        end++;
        start = end;
    }
    return;
}

int getCV_Lock_Mapping(int CVID) {
    int LockId = -1;
    LockId = CVID + LOCK_OFFSET;
    return LockId;
}

/* Data Update Handling Functions
int UpdateData_Patient(Packet p) {

    return 0;
}

int UpdateData_Receptionist(Packet p) {
    int id, peopleInLine, currentToken;
    id = copyOutInt(p.data, 0);
    peopleInLine = copyOutInt(p.data, 4);
    currentToken = copyOutInt(p.data, 8);

    receptionists[id].currentToken = currentToken;
    receptionists[id].peopleInLine = peopleInLine;
    return id;
}
int UpdateData_Doorboy(Packet p) {

    return 0;
}
int UpdateData_Doctor(Packet p) {
    int id, peopleInLine, prescription, patientToken;
    id = copyOutInt(p.data, 0);
    prescription = copyOutInt(p.data, 4);
    patientToken = copyOutInt(p.data, 8);

    doctors[id].patientToken = patientToken;
    doctors[id].peopleInLine = peopleInLine;
    doctors[id].prescription = prescription;
    return id;
}

int UpdateData_Cashier(Packet p) {
    int id, lineLength, patToken, fee, payment, sales;
    id = copyOutInt(p.data, 0);
    lineLength = copyOutInt(p.data, 4);
    patToken = copyOutInt(p.data, 8);
    fee = copyOutInt(p.data, 12);
    payment = copyOutInt(p.data, 16);
    sales = copyOutInt(p.data, 20);

    cashiers[id].lineLength = lineLength;
    cashiers[id].patToken = patToken;
    cashiers[id].fee = fee;
    cashiers[id].payment = payment;
    cashiers[id].sales = sales;
    return id;
}

int UpdateData_Clerk(Packet p) {
    int id, patientsInLine, payment, fee, patPrescription, sales;
    id = copyOutInt(p.data, 0);
    patientsInLine = copyOutInt(p.data, 4);
    payment = copyOutInt(p.data, 8);
    fee = copyOutInt(p.data, 12);
    patPrescription = copyOutInt(p.data, 16);
    sales = copyOutInt(p.data, 20);

    clerks[id].patientsInLine = patientsInLine;
    clerks[id].payment = payment;
    clerks[id].fee = fee;
    clerks[id].patPrescription = patPrescription;
    clerks[id].sales = sales;
    return id;
}

int UpdateData_HospitalManager(Packet p) {

    return 0;
}

int UpdateData_Global(Packet p) {
    int status = -1;
    short variableToUpdate = 0x00;
    int value = -1;
    int key = -1;
    variableToUpdate = copyOutShort(p.data, 0);
    switch (variableToUpdate) {
        case NUMDOCTORS:
            value = copyOutInt(p.data, 2);
            numDoctors = value;
            break;
        case NUMCASHIERS:
            value = copyOutInt(p.data, 2);
            numCashiers = value;
            break;
        case NUMCLERKS:
            value = copyOutInt(p.data, 2);
            numClerks = value;
            break;
        case NUMDOORBOYS:
            value = copyOutInt(p.data, 2);
            numDoorboys = value;
            break;
        case NUMRECP:
            value = copyOutInt(p.data, 2);
            numRecp = value;
            break;
        case NUMPATIENTS:
            value = copyOutInt(p.data, 2);
            numPatients = value;
            break;
        case FEESPAID:
            value = copyOutInt(p.data, 2);
            feesPaid = value;
            break;
        case TEST_STATE:
            value = copyOutInt(p.data, 2);
            test_state = value;
            break;
        case TOKENCOUNTER:
            value = copyOutInt(p.data, 2);
            TokenCounter = value;
            break;
        case TOTALSALES:
            value = copyOutInt(p.data, 2);
            totalsales = value;
            break;
        case PEOPLEINHOSPITAL:
            value = copyOutInt(p.data, 2);
            peopleInHospital = value;
            break;
        case DOORBOYLINELENGTH:
            value = copyOutInt(p.data, 2);
            doorboyLineLength = value;
            break;
        case PATCOUNT:
            value = copyOutInt(p.data, 2);
            patientCount = value;
            break;
        case RECPCOUNT:
            value = copyOutInt(p.data, 2);
            recptionistCount = value;
            break;
        case DOORBCOUNT:
            value = copyOutInt(p.data, 2);
            doorboyCount = value;
            break;
        case DOCCOUNT:
            value = copyOutInt(p.data, 2);
            doctorCount = value;
            break;
        case CASHCOUNT:
            value = copyOutInt(p.data, 2);
            cashierCount = value;
            break;
        case CLERKCOUNT:
            value = copyOutInt(p.data, 2);
            pharmacyCount = value;
            break;
        case HOSPMANCOUNT:
            value = copyOutInt(p.data, 2);
            hospitalmanagerCount = value;
            break;
        case FEELIST_APPEND:
            key = copyOutInt(p.data, 2);
            value = copyOutInt(p.data, 6);
            List_Append(&feeList, key, value);
            break;
        case QUEUE_PUSH:
            value = copyOutInt(p.data, 2);
            Queue_Push(&wakingDoctorList, value);
            break;
        case QUEUE_POP:
            Queue_Pop(&wakingDoctorList);
            break;
        default:
            break;
    }
    return variableToUpdate;
}
*/
int DistUpdate_Send(Packet p) {
    /* This function is used to broadcast an Update packet to every one
     */
    int senderMBox = 0;
    int i, j;
    for (j = 0; j < 7; j++)
        for (i = 0; i < numberOfEntities[j]; i++)
            Packet_Send(j, i + 1, myMbox, &p);
}

/* Hospital Entity interface for the Network Data Update */
int HDataUpdate_Recp(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Receptionist(&p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, &p);
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
int HDataUpdate_Doc(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Doctor(&p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, &p);
    return status;
}
int HDataUpdate_Cash(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Cashier(&p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, &p);
    return status;

}
int HDataUpdate_Clerk(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Clerk(&p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, &p);
    return status;

}
int HDataUpdate_HospMan(int id) {
    int status = -1;
    return status;
}

int HGlobalDataUpdate(short Variable, int val) {
    int status = -1;
    Packet p;
    buildPacket_GlobalData(p, Variable, val);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, &p);
    return status;
}

int HGlobalListAppendUpdate(int key, int val) {
    int status = -1;
    Packet p;
    buildPacket_GlobalListAppend(&p, key, val);
    status = Packet_Send(GetMachineID(), myNetThreadMbox, 0, &p);
    return status;
}

int HGlobalQueuePopUpdate() {
    int status = -1;
    Packet p;
    buildPacket_GlobalQueuePop(&p);
    status = Packet_Send(GetMachineID(), myNetThreadMbox, 0, &p);
    return status;
}

int HGlobalQueuePushUpdate(int value) {
    int status = -1;
    Packet p;
    buildPacket_GlobalQueuePush(&p, value);
    status = Packet_Send(GetMachineID(), myNetThreadMbox, 0, &p);
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

/******************************
 ****** SYSTEM INITIALIZE *****
 ******************************/
void initializeSystem() {
    /* This function takes care of initializing the various
     * locks and other things at the system startup
     */
    /* Read all the configuration data */
    readConfig();
    myNetThreadMbox = getMyNetThreadMbox();
    /* create system locks and CV*/
    netthread_Lock = CreateLock("netthread_Lock");
    netthread_CV = CreateCondition("netthread_CV");

}
