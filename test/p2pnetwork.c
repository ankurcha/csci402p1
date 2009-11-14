#include "p2pnetwork.h"
char *Serialize(Packet& p, char *message){
    unsigned int i=0;
    message[1] = p.senderId;
    message[0] = p.senderId >> 8;
    message[3] = p.timestamp;
    message[2] = p.timestamp >> 8;
    for( i=4; i<MaxMailSize; i++)
        message[i] = data[i-4];
    return message;
}

int Deserialize(Packet &p, char* data){
    unsigned int i = 0;
    if(p == NULL)
        return -1;
    p.senderId = (int)((message[0]*256)+message[1]);
    p.timestamp = (int)((message[2]*256)+message[3]);
    for(i=4;i<MaxMailSize; i++)
        data[i-4] = message[i];
    return 0;
}

/* Lock Functions */

int HLock_Create(int HlockId, int EntityID){
    LockId LID = -1;
    int SendStatus = -1;
    char LockMessage[MaxDataSize]; /* Message to be sent */
    if(HlockId <= -1)
        return -1;
    /* Broadcast a create a message to all the different registered nodes which
     * have replied NODE_READY 
     * We pass this message to the broadcast call and forget
    */

    /* Create the lock and confirm it up else return -1 */
    LID = CreateLock("Lock");
    if(LID < 0)
        return -1;

    /* Successfully created a lock now we create a message */
    /* Operation type */
    LockMessage[0] = LOCK_CREATE;
    /* Operation detail is empty for locks */
    LockMessage[1] = EMPTY;
    /* Lock ID */
    LockMessage[2] = EntityID >> 8;
    LockMessage[3] = EntityID;

    /* Now send this message */
    SendStatus = Hospital_Send(LockMessage);
    return SendStatus;
}


