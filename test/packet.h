/* Library to provide the packet data type
 *
 * USC CS 402 Fall 2009
 * Max Pflueger
 */

#ifndef PACKET_H
#define PACKET_H 1

/* Easy access to the data in a message */
struct packet {
    int senderId;
    int timestamp;
    char packetType;
    char data[MaxMailSize - DATA];
};
typedef struct packet Packet;

#endif /*PACKET_H*/


