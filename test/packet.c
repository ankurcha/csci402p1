/* Library implementation to provide the packet data type
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#include "packet.h"
void printHex(char *message, int len){
    int i = 0;
    char a;
    char str[2*MaxMailSize+1];
    if(len > MaxMailSize){
        print("LEN must be 40\n");
        len = MaxMailSize;
    }

    for(i=0;i<len; i++){
        a = message[i];
        if((a>>4) < 10){
            str[2*i] = (a>>4) + '0';
        }else{
           str[2*i] = (a>>4) + 'A';
        }
       if((a%16) <10){
            str[2*i+1] = (a%16) + '0';
        }else{
           str[2*i+1] = (a%16) + 'A';
        }
    }
    str[2*i] = 0;
    print(str);
}
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
    status = Receive(mbox, senderId, senderMBox, data);

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
    copyInShort(message, NAME, p->senderId);
    copyInInt(message, TIMESTAMP, p->timestamp);
    message[PACKET_TYPE] = p->packetType;
    copyInData(message, DATA, p->data, MaxMailSize - DATA);
    /*
    print("Ser: ");
    printHex(message, 40);
    print("\n");
    */
}

void DeserializePacket(Packet *p, char* message) {
    /*
    print("De-ser: ");
    printHex(message, 40);
    print("\n");
    */
    p->senderId = copyOutShort(message, NAME);
    p->timestamp = copyOutInt(message, TIMESTAMP);
    p->packetType = message[PACKET_TYPE];
    copyOutData(message, DATA, p->data, MaxMailSize - DATA);
}

/* deep copy memory y to x */
void memcopy(char* x, char* y, int length) {
    int i=0;
    for(i=0; i < length; i++) {
        *(x+i) = *(y+i);
    }
}


