/*
 * hospShared.c
 *
 *  Created on: Nov 22, 2009
 *      Author: ankurcha
 */
/*
 * Packets for the shared data updates.
 */
#include "hospShared.h"

Packet *buildPacket_Receptionist(Packet *p, int id, int peopleInLine,
        int currentToken) {
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = RECP_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, peopleInLine);
    copyInInt(p->data, 8, currentToken);
    return p;
}

Packet *buildPacket_Doctor(Packet *p, int id, int peopleInLine, int prescription, int patientToken) {
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = DOC_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, peopleInLine);
    copyInInt(p->data, 8, prescription);
    copyInInt(p->data, 12, patientToken);
    return p;
}

Packet *buildPacket_Cashier(Packet *p, int id, int lineLength, int patToken, int fee, int payment, int sales) {
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = CASH_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, lineLength);
    copyInInt(p->data, 8, patToken);
    copyInInt(p->data, 12, fee);
    copyInInt(p->data, 16, payment);
    copyInInt(p->data, 20, sales);
    return p;

}

Packet *buildPacket_Clerk(Packet *p, int id, int patientsInLine, int payment, int fee, int patPrescription, int sales) {
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = CLERK_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, patientsInLine);
    copyInInt(p->data, 8, payment);
    copyInInt(p->data, 12, fee);
    copyInInt(p->data, 16, patPrescription);
    copyInInt(p->data, 20, sales);
    return p;
}

Packet *buildPacket_GlobalData(Packet *p, short variable, int val) {
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = GLOBAL_DATA_UPDATE;
    copyInShort(p->data, 0, variable);
    copyInInt(p->data, 2, val);
    return p;
}

Packet *buildPacket_GlobalListAppend(Packet *p, int key, int val) {
    int variable = FEELIST_APPEND;
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = GLOBAL_DATA_UPDATE;
    copyInShort(p->data, 0, variable);
    copyInInt(p->data, 2, key);
    copyInInt(p->data, 6, val);
    return p;
}

Packet *buildPacket_GlobalQueuePush(Packet *p, int val) {
    return buildPacket_GlobalData(p, QUEUE_PUSH, val);
}
Packet *buildPacket_GlobalQueuePop(Packet *p) {
    int variable = QUEUE_POP;
    p->senderId = GetMachineID();
    p->timestamp = GetTimestamp();
    p->packetType = GLOBAL_DATA_UPDATE;
    copyInShort(p->data, 0, variable);
    /* No Data here */
    return p;
}
