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
#endif /* HOSPSHARED_H_ */
