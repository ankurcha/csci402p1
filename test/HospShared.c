/*
 * HospShared.c
 *
 *  Created on: Nov 22, 2009
 *      Author: ankurcha
 */
/*
 * Packets for the shared data updates.
 */

#include "p2pnetwork.h"

Packet* buildPacket_Receptionist(Packet *p, int id) {
    p->senderId = GetMachineId();
    p->timestamp = GetTimestamp();
    p->packetType = RECP_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, receptionists[id].peopleInLine);
    copyInInt(p->data, 8, receptionists[id].currentToken);
    return p;
}

Packet *buildPacket_Doctor(Packet *p, int id) {
    p->senderId = GetMachineId();
    p->timestamp = GetTimestamp();
    p->packetType = DOC_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, doctors[id].peopleInLine);
    copyInInt(p->data, 8, doctors[id].prescription);
    copyInInt(p->data, 12, doctors[id].patientToken);
    return p;
}

Packet *buildPacket_Cashier(Packet *p, int id) {
    p->senderId = GetMachineId();
    p->timestamp = GetTimestamp();
    p->packetType = CASH_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, cashiers[id].lineLength);
    copyInInt(p->data, 8, cashiers[id].patToken);
    copyInInt(p->data, 12, cashiers[id].fee);
    copyInInt(p->data, 16, cashiers[id].payment);
    copyInInt(p->data, 20, cashiers[id].sales);
    return p;

}

Packet *buildPacket_Clerk(Packet *p, int id) {
    p->senderId = GetMachineId();
    p->timestamp = GetTimestamp();
    p->packetType = CLERK_DATA_UPDATE;
    copyInInt(p->data, 0, id);
    copyInInt(p->data, 4, clerks[id].patientsInLine);
    copyInInt(p->data, 8, clerks[id].payment);
    copyInInt(p->data, 12, clerks[id].fee);
    copyInInt(p->data, 16, clerks[id].patPrescription);
    copyInInt(p->data, 20, clerks[id].sales);
    return p;
}

Packet *buildPacket_GlobalData(Packet *p, short variable, int val) {
    p->senderId = GetMachineId();
    p->timestamp = GetTimestamp();
    p->packetType = GLOBAL_DATA_UPDATE;
    copyInShort(p->data, 0, variable);
    copyInInt(p->data, 2, val);
    return p;
}
