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

/* Communication Functions */

int Hospital_Receive(int senderId, int mailboxId, Packet &receivedPacket){
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
    status = Receive(senderId, mailboxId, data);
    /* Deserialize the data to get the packet into receivedPacket*/
    if( status != -1)
        status = Deserialize(receivedPacket, data);
    return status;
}

int Hospital_Send(int receiverId, int mailboxId, Packet& p){
    /* This function actually sends data to receiverId:mailboxId
     * after serializing the data into a packet which can be received
     * at the other end using Hospital_Receive(...) function
     */
    int status = -1;
    char message[MaxMailSize];
    message = Serialize(p, message); 
    /* Now that we have the packet serialized in message,
     * do a send till we are sure that this host receives it,
     **************** THIS IS A BLOCKING SEND ********************
     */
    status = -1;
    while(status == -1)
        status = Send(receiverId, mailboxId, message);
    /* Finally, We were able to successfully send to the receiver host
     * now just return status to the calling function
     * TODO: Do we need a time out???
    */
    return status;
}

int Hospital_Multicast(int hostList[],int mailboxList[], int NumberOfHosts, Packet& p){
    /* This function should "magically" get the list of the hosts to
     * send the messages to, In case sending to any of the hosts fails,
     * return -1 else returns the number of the hospitals that actually
     * got the message.
     **************** THIS IS A BLOCKING SEND **********************
    */
    int curstatus = -1;
    int status = 0;
    int i = 0;
    for(i=0;i < numberOfHosts; i++){
        curstatus = Hospital_Send(hostList[i], mailboxList[i], p);
        if(curstatus == -1)
            return -1;
        else
            status++;
    }
    return status;
}
/* Lock Functions */
int HLock_Acquire(int HlockId){
    /* Check the validity of the HlockId and EntityId */

    /* Create a lock query */
    /* Send the SYN to the network entity for Lock Acquire*/
    /* Wait for an ACK */
    /* Received and ACK now just return the value received */
}

int HLock_Release(int HlockId){
    /* Check if I already own this lock */
    /* Send message to announce release of the lock to the network entity */
    /* Wait for ACK from the network entity */
    /* Return success */
}
