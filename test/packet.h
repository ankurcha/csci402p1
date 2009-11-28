/* Library to provide the packet data type
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef PACKET_H
#define PACKET_H 1

#define MaxMailSize 40

/* Define our message structure */
enum {
    NAME = 0, TIMESTAMP = 2, PACKET_TYPE = 6, DATA = 7
};

/* Easy access to the data in a message */
struct packet {
    int senderId;
    unsigned int timestamp;
    unsigned char packetType;
    unsigned char data[MaxMailSize - DATA];
};
typedef struct packet Packet;

/* These system calls for doing the netowork I/O
 * This also takes care of the protocol stack.
 */

int Packet_Receive(int mbox, int *senderId, int *senderMBox,
        Packet *receivedPacket);

int Packet_Send(int receiverId, int recMBox, int senderMBox, Packet*);

void SerializePacket(Packet *p, unsigned char* message);
void DeserializePacket(Packet *p, unsigned char* message);

/* Defined some methods for putting data into and out of messages */
int copyOutInt(unsigned char* message, int index);
void copyInInt(unsigned char* message, int index, int val);

int copyOutShort(unsigned char* message, int index);
void copyInShort(unsigned char* message, int index, int val);

void copyOutData(unsigned char* message, int index, unsigned char* data, int length);
void copyInData(unsigned char* message, int index, unsigned char* data, int length);

/* copies right to left */
void memcopy(char* left, char* right, int length);

#endif /*PACKET_H*/

