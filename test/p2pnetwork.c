#include "p2pnetwork.h"
#include "print.h"

int copyOutInt(char* message, int index) {
    int val = 0;
    unsigned int i = 0;

    for(i=0; i < 4; i++) {
        val = (val << 8) + message[i + index];
    }

    return val;
}

void copyInInt(char* message, int index, int val) {
    unsigned int i = 0;

    for(i=0; i < 4; i++) {
        message[i + index] = (val >> (8 * (3-i))) & 0xFF;
    }
}

int copyOutShort(char* message, int index) {
    int val = 0;
    unsigned int i = 0;

    for(i=0; i < 2; i++) {
        val = (val << 8) + message[i + index];
    }

    return val;
}

void copyInShort(char* message, int index, int val) {
    unsigned int i = 0;

    for(i=0; i < 2; i++) {
        message[i + index] = (val >> (8 * (1-i))) & 0xFF;
    }
}

void copyOutData(char* message, int index, char* data, int length) {
    unsigned int i=0;

    if(index + length > MaxMailSize) {
        print("ERROR: copyOutData beyond end of message\n");
        return;
    }

    for( i=0; i<length; i++) {
        data[i] = message[i + index];
    }
}

void copyInData(char* message, int index, char* data, int length) {
    unsigned int i=0;

    if(index + length > MaxMailSize) {
        print("ERROR: copyInData beyond end of message\n");
        return;
    }

    for( i=0; i<length; i++) {
        message[i + index] = data[i];
    }
}

/*char *Serialize(Packet& p, char *message){
    unsigned int i=0;
    message[1] = p.senderId;
    message[0] = p.senderId >> 8;
    message[3] = p.timestamp;
    message[2] = p.timestamp >> 8;
    for( i=4; i<MaxMailSize; i++)
        message[i] = data[i-4];
    return message;
}
*/

void SerializePacket(Packet& p, char* message) {
    copyInShort(message, SENDER_ID, p.senderId);
    copyInInt(message, TIMESTAMP, p.timestamp);
    message[PACKET_TYPE] = p.packetType;
    copyInData(message, DATA, p.data, MaxMailSize - DATA);
}

void DeserializePacket(Packet& p, char* message) {
    p.senderId = copyOutShort(message, SENDER_ID);
    p.timestamp = copyOutInt(message, TIMESTAMP);
    p.packetType = message[PACKET_TYPE];
    copyOutData(message, DATA, p.data, MaxMailSize - DATA);
}

/*int Deserialize(Packet &p, char* data){
    unsigned int i = 0;
    if(p == NULL)
        return -1;
    p.senderId = (int)((message[0]*256)+message[1]);
    p.timestamp = (int)((message[2]*256)+message[3]);
    for(i=4;i<MaxMailSize; i++)
        data[i-4] = message[i];
    return 0;
}
*/

int addHeldResource(int type, int id){
    int i=0;
    int targetpos = -1;
    for(i=0;i<MAX_RESOURCES;i++)
        if(resourcesHeld[i].valid = 0){
            targetpos = i;
            break;
        }
    if(targetPos == -1)
        return -1;
    resourcesHeld[targetPos].resourceType = type;
    resourcesHeld[targetPos].resourceID = id;
    resourcesHeld[targetPos].valid = 1;
    return targetPos;
}

void initResourcesHeldList(){
    int i = 0;
    for(i=0;i<MAX_RESOURCES;i++)
        resourcesHeld[i].valid = 0;
}

int releaseResource(int type, int id){
    int i = 0;
    for(i =0; i<MAX_RESOURCES;i++){
        if(resourcesHeld[i].resourceType == type && 
           resourcesHeld[i].resourceId == id){
            resourcesHeld[i].valid = 0;
            return 1;
        }
    }
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
    int status = -1;
    /* Check if I already own this lock */
    if(HlockId <0)
        return status;
    /* Create message for Lock Release */
    Packet p;
    p.senderId = GetMachineId();
    p.timestamp = GetTimestamp();
    p.data[0] = LOCK_RELEASE;
    p.data[1] = EMPTY;
    p.data[2] = data[3] = EMPTY;
    p.data[4] = HlockId>>8;
    p.data[5] = HlockId;
    /* Send message to announce release of the lock to the network entity */
    status = ScheduleForSend(p);
    /* Check for successful Multicast */
    if(status > -1)
        status = 0;
    return status;
    /* Return success */
}

int HLock_Acquire(int HlockId){
    int status = -1;
    /* For this operation we need to do a distributed concensus and then decide
     * who should get the lock, else we just keep waiting till we do get
     */

    p.senderId = GetMachineId();
    p.timestamp = GetTimestamp();
    p.data[0] = LOCK_ACQUIRE;
    data[1] = data[2] = data[3] = EMPTY;
    data[4] = HlockId>>8;
    data[5] = HlickId;
    
    /* We have now built the packet and now we should do the following  
    */
}

void readConfig(){
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
    fd = Open("configfile",10);
    bytesread = Read(buf, 100, fd);
    Close(fd);
    end = 0;
    for (k = 0; k<7; k++) {
        /* Parse the comma separated numbers */
        for (i=0; i<bytesread; i++) {
            if(buf[i] != ','){
                end++;
            }
        }
        /* copy the number string to temp */
        for(j=start;j<end;j++)
            temp[j-start] = buf[j];
        print("Reading Config:"); 
        print(temp);
        print("\n");
        numberOfEntities[k] = atoi(temp);
        end++;
        start = end;
    }
    return;
}
