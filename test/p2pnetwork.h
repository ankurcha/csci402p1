
#ifndef P2PNETWORK_H
#define P2PNETWORK_H 1

#inlcude "syscall.h"
#define MaxMailSize 40
/*#define MaxDataSize (MaxMailSize - 4)*/

#define MAX_RESOURCES 50

int numberOfEntities[7]; /* The number of entities */
/*
 * These system calls for doing the netowork I/O
 * This also takes care of the protocol stack.
 *
 */

int Packet_Receive(int mbox, 
                   int& senderId, 
                   int& senderMBox, 
                   Packet &receivedPacket);

int Packet_Send(int receiverId, int recMBox, senderMBox, Packet&);

int[] getHostList();
int[] getMailboxList();
int GetNumberOfHosts();

/*
 * We need to keep a list of the resources that we currently hold
 * so, all the locks and CVs that we hold are with us are present in
 * this list of resources
 */
struct Resource{
    int resourceType; /* 0 - Lock , 1 - CV */
    int resourceID;
    char valid; /* 0 init */
};

typedef struct Resource Resource;

Resource resourcesHeld[MAX_RESOURCES]
void initResourcesHeldList();
int addHeldResource(int type, int id);
int releaseResource(int type, int id);

/*
 * Packet structure
 * The data portion of the packet has fields. Each field is 1 byte in length
 * data[0] - type of operation
 *           LOCK_CREATE = 0x01
 *           LOCK_DESTROY = 0x02
 *           LOCK_ACQUIRE = 0x03
 *           LOCK_RELEASE = 0x04
 *           CV_CREATE = 0x05
 *           CV_DESTROY = 0x06
 *           CV_WAIT = 0x07
 *           CV_SIGNAL = 0x08
 *           CV_BROADCAST = 0x09
 *           NODE_START = 0x0A
 *           NODE_STOP = 0x0B
 *           NODE_READY = 0x0C
 * data[1] - Operation detail
 *           For NODE Operations
 *              RECEPTIONIST_NODE = 0x01
 *              PATIENT_NODE = 0x02
 *              DOCTOR_NODE = 0x03
 *              DOORBOY_NODE = 0x04
 *              CASHIER_NODE = 0x05
 *              CLERK_NODE = 0x06
 *              MANAGER_NODE = 0x07
 *          For LOCK Operations
 *              EMPTY - 0x00 All Locks Are the same
 *          For CV Operations
 *              EMPTY - 0x00 All Condition Variables are the same
 * data[2,3,4,5,6,7] - Entity Id (sized int hence 2 bytes)
 *          For NODE - Node ID
 *          For Lock - Lock ID
 *          For CV   - CV ID
 */
enum {
    EMPTY = 0x00,
    LOCK_CREATE = 0x01,
    LOCK_DESTROY = 0x02,
    LOCK_ACQUIRE = 0x03,
    LOCK_RELEASE = 0x04,
    CV_CREATE = 0x05,
    CV_DESTROY = 0x06,
    CV_WAIT = 0x07,
    CV_SIGNAL = 0x08,
    CV_BROADCAST = 0x09,
    NODE_START = 0x0A,
    NODE_STOP = 0x0B,
    NODE_READY = 0x0C,
    LOCAL_ENTITY = 0x0D
};
/* Entity Identification */
enum {
    RECEPTIONIST_NODE = 0x0D,
    PATIENT_NODE = 0x0E,
    DOCTOR_NODE = 0x0F,
    DOORBOY_NODE = 0x10,
    CASHIER_NODE = 0x11,
    CLERK_NODE = 0x12,
    MANAGER_NODE = 0x13
};

/* Define our message structure */
enum {
    SENDER_ID = 0;
    TIMESTAMP = 2;
    PACKET_TYPE = 6;
    DATA = 7;
}

/* Easy access to the data in a message */
struct packet{
    int senderId;
    int timestamp;
    char packetType;
    char data[MaxMailSize - DATA];
};
typedef struct packet Packet;

/*char *SerializePacket(Packet&, char*, int senderId);*/
void SerializePacket(Packet& p, char* message);
void DeserializePacket(Packet& p, char* message);
/*int DeserializePacket(Packet&, char*, int senderId);*/

/* Defined some methods for putting data into and out of messages */
int copyOutInt(char* message, int index);
void copyInInt(char* message, int index,  int val);

int copyOutShort(char* message, int index);
void copyInShort(char* message, int index, int val);

void copyOutData(char* message, int index, char* data, int length);
void copyInData(char* message, int index, char* data, int length);


/*********************************
 ******** MUTUAL EXCLUSION *******
 *********************************/

/* Register/Acquire/Release/Destroy a lock with the distributed system 
 * causes all to become aware that this lock is available for messing with
 */
int HLock_Acquire(int HlockId);
int HLock_Release(int HlockId);

/* Register/Signal/Wait/Broadcast/Destroy a Condition variable with the
 * distributed system
 */
int HCV_Signal(int HCVId, int HLockId);
int HCV_Wait(int HCVId, int HLockId);
int HCV_Broadcast(int HCVId, int HLockId);

/* A synchronized print routine to print the lines on the console nicely
 */
int SyncPrint(char *, int EntityID);

/* Update the shared memory elements */

int HTable_Update(int tableId, int key, int value);

/* Send a message to network */
int SendToNetwork();

/* misc functions:
 * 1. Read configuration file
 */

int readConfig(char *filename);
#endif /* P2PNETWORK_H */

