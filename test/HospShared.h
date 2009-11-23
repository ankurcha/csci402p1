/*
 * HospShared.h
 *
 *  Created on: Nov 22, 2009
 *      Author: ankurcha
 */

#ifndef HOSPSHARED_H_
#define HOSPSHARED_H_

/* Data Update packet building functions */
Packet* buildPacket_Receptionist(Packet *p, int id, int peopleInLine,
        int currentToken);
Packet *buildPacket_Doctor(Packet *p, int id, int peopleInLine,
        int prescription, int patientToken);
Packet *buildPacket_Cashier(Packet *p, int id, int lineLength, int patToken,
        int fee, int payment, int sales);
Packet *buildPacket_Clerk(Packet *p, int id, int patientsInLine, int payment,
        int fee, int patPrescription, int sales);
Packet *buildPacket_GlobalData(Packet *p, short variable, int val);
Packet *buildPacket_GlobalListAppend(Packet *p, short variable, int key,
        int val);
Packet *buildPacket_GlobalQueuePush(Packet *p, int val);
Packet *buildPacket_GlobalQueuePop(Packet *p);
#endif /* HOSPSHARED_H_ */
