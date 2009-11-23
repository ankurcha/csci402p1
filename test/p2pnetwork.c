#include "p2pnetwork.h"
#include "print.h"

/* Communication Functions */

int Packet_Receive(int mbox, int* senderId, int* senderMBox,
        Packet *receivedPacket) {
    /* This function performs a read and put the fields in the respective places
     * receivedPacket.senderId - reference to senderId
     * receivedPacket.timestamp - timestamp of the received message
     * receivedPacket.data - data received
     *
     * returns -1 of failure
     */

    int status = -1;
    char data[MaxMailSize];

    /* actually get the data into the char *data variable */
    status = Receive(mbox, &senderId, &senderMBox, data);

    /* Deserialize the data to get the packet into receivedPacket*/
    if (status != -1) {
        DeserializePacket(receivedPacket, data);
    }

    return status;
}

int Packet_Send(int receiverId, int recMBox, int senderMBox, Packet* p) {
    /* This function actually sends data to receiverId:mailboxId
     * after serializing the data into a packet which can be received
     * at the other end using Hospital_Receive(...) function
     */

    int status = -1;
    char message[MaxMailSize];
    SerializePacket(p, message);

    /* Now that we have the packet serialized in message,
     * do a send till we are sure that this host receives it,
     **************** THIS IS A BLOCKING SEND ********************
     */

    status = -1;
    while (status == -1) {
        status = Send(receiverId, recMBox, senderMBox, message);
    }

    /* Finally, We were able to successfully send to the receiver host
     * now just return status to the calling function
     */

    return status;
}

/*****************************************
 ******* PACKET MANIPULATION *************
 *****************************************/

int copyOutInt(char* message, int index) {
    int val = 0;
    unsigned int i = 0;

    for (i = 0; i < 4; i++) {
        val = (val << 8) + message[i + index];
    }

    return val;
}

void copyInInt(char* message, int index, int val) {
    unsigned int i = 0;

    for (i = 0; i < 4; i++) {
        message[i + index] = (val >> (8 * (3 - i))) & 0xFF;
    }
}

int copyOutShort(char* message, int index) {
    int val = 0;
    unsigned int i = 0;

    for (i = 0; i < 2; i++) {
        val = (val << 8) + message[i + index];
    }

    return val;
}

void copyInShort(char* message, int index, int val) {
    unsigned int i = 0;

    for (i = 0; i < 2; i++) {
        message[i + index] = (val >> (8 * (1 - i))) & 0xFF;
    }
}

void copyOutData(char* message, int index, char* data, int length) {
    unsigned int i = 0;

    if (index + length > MaxMailSize) {
        print("ERROR: copyOutData beyond end of message\n");
        return;
    }

    for (i = 0; i < length; i++) {
        data[i] = message[i + index];
    }
}

void copyInData(char* message, int index, char* data, int length) {
    unsigned int i = 0;

    if (index + length > MaxMailSize) {
        print("ERROR: copyInData beyond end of message\n");
        return;
    }

    for (i = 0; i < length; i++) {
        message[i + index] = data[i];
    }
}

void SerializePacket(Packet *p, char* message) {
    copyInShort(message, SENDER_ID, p.senderId);
    copyInInt(message, TIMESTAMP, p.timestamp);
    message[PACKET_TYPE] = p.packetType;
    copyInData(message, DATA, p.data, MaxMailSize - DATA);
}

void DeserializePacket(Packet *p, char* message) {
    p.senderId = copyOutShort(message, SENDER_ID);
    p.timestamp = copyOutInt(message, TIMESTAMP);
    p.packetType = message[PACKET_TYPE];
    copyOutData(message, DATA, p.data, MaxMailSize - DATA);
}

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
    /*TODO: Send a lock acquire message to all the targets */
    /* Add the requested resource to the requestedResource Array */
    addResource(resourcesRequested, LOCK, temp, 0);
    for (j = 0; j < 7; j++) {
        for (int i = 0; i < numberOfEntities[j]; i++) {
            /* Sending LOCK_ACQUIRE to all and waiting for LOCK_OK */
            Packet_Send(j, i+1, myMbox, pkt);
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
                Packet_Send(j, i+1, myMbox, pkt);
            }
        }
        return 1; /* successfully released lock */
    }
}

int RemoteLock_Acquire(Packet pkt) {

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
            Packet_Send(j, i+1, myMbox, p);
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
            Packet_Send(j, i+1, myMbox, p);
        }
    }

    if(senderId == GetMachineId() && senderMBox == myMbox) {
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

    if(senderId == GetMachineId() && senderMBox == myMbox) {
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
    /* Do some magical mapping */
    return LockId;
}

/* Data Update Handling Functions */
int UpdateData_Patient(Packet p) {
    /* !!! */
    return 0;
}

int UpdateData_Receptionist(Packet p) {
    int id, peopleInLine, currentToken;
    id = copyOutInt(p.data, 0);
    peopleInLine = copyOutInt(p.data, 4);
    currentToken = copyOutInt(p.data, 8);
    /* Apply the update */
    receptionists[id].currentToken = currentToken;
    receptionists[id].peopleInLine = peopleInLine;
    return id;
}
int UpdateData_Doorboy(Packet p) {
    /* !!! */
    return 0;
}
int UpdateData_Doctor(Packet p) {
    int id, peopleInLine, prescription, patientToken;
    id = copyOutInt(p.data, 0);
    prescription = copyOutInt(p.data, 4);
    patientToken = copyOutInt(p.data, 8);
    /* Apply update */
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
    /* Apply Update */
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
    /* Apply Update */
    clerks[id].patientsInLine = patientsInLine;
    clerks[id].payment = payment;
    clerks[id].fee = fee;
    clerks[id].patPrescription = patPrescription;
    clerks[id].sales = sales;
    return id;
}

int UpdateData_HospitalManager(Packet p) {
    /* !!! */
    return 0;
}
int DistUpdate_Send(Packet p) {
    /* This function is used to broadcast an Update packet to every one
     */
    int senderMBox = 0;
    int i, j;
    for (j = 0; j < 7; j++)
        for (i = 0; i < numberOfEntities[j]; i++)
            Packet_Send(receiverId, recMBox, senderMBox, p);
}

/* Hospital Entity interface for the Network Data Update */
int HDataUpdate_Recp(int id) {
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Receptionist(p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
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
int HDataUpdate_Doc(int id){
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Doctor(p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
    return status;
}
int HDataUpdate_Cash(int id){
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Cashier(p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
    return status;

}
int HDataUpdate_Clerk(int id){
    /* Creates an update packet and sends it to the network entity for processing */
    Packet p;
    int status = -1;
    buildPacket_Clerk(p, id);
    status = Packet_Send(GetMachineId(), myNetThreadMbox, 0, p);
    return status;

}
int HDataUpdate_HospMan(int id){
    int status = -1;
    return status;
}

/******************************
 ****** SYSTEM INITIALIZE *****
 ******************************/
void initializeSystem(){
    /* This function takes care of initializing the various
     * locks and other things at the system startup
     */
    /* Read all the configuration data */
    readConfig();
    /* create system locks and CV*/
    netthread_Lock = CreateLock("netthread_Lock");
    netthread_CV = CreateCondition("netthread_CV");

}
