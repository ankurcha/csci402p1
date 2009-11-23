/*
 * hospShared.h
 *
 *  Created on: Nov 22, 2009
 *      Author: ankurcha
 */

#ifndef HOSPSHARED_H_
#define HOSPSHARED_H_

#include "init.h"

/* Data Update packet building functions */
Packet* buildPacket_Receptionist(Packet *p, int id);
Packet *buildPacket_Doctor(Packet *p, int id);
Packet *buildPacket_Cashier(Packet *p, int id);
Packet *buildPacket_Clerk(Packet *p, int id);
Packet *buildPacket_GlobalData(Packet *p, short variable, int val);
Packet *buildPacket_GlobalListAppend(Packet *p, int key, int val);
Packet *buildPacket_GlobalQueuePush(Packet *p, int val);
Packet *buildPacket_GlobalQueuePop(Packet *p);
#endif /* HOSPSHARED_H_ */
