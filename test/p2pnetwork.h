#ifndef P2PNETWORK_H
#define P2PNETWORK_H 1

#include "syscall.h"
#define MaxMailSize 40
#define MAX_CV 50
#define MAX_CV_QUEUE_LEN 100
#define MaxEntities 50
/*#define MaxDataSize (MaxMailSize - 4)*/

#define MAX_RESOURCES 50

/************************************
 ******** GLOBAL DATA ***************
 ************************************/

int numberOfEntities[7]; /* The number of entities */
/* numberOfEntities[0] - patients
 * numberOfEntities[1] - receptionists
 * numberOfEntities[2] - doorboy
 * numberOfEntities[3] - doctors
 * numberOfEntities[4] - cashiers
 * numberOfEntities[5] - clerks
 * numberOfEntities[6] - hospital managers
 */

int netthread_Lock; /* We use these to interact with the netthread */
int netthread_CV; /* Used to wait on the netthread for a reply */

/* We need to keep a list of the resources that we currently hold
 * so, all the locks and CVs that we hold are with us are present in
 * this list of resources
 */

struct Resource {
    char name[20];
    int timestamp; /* record when the request was made */
    char valid; /* 0 init */
    int replies; /* Number of replies that are received */
    int state; /* can be RES_HELD or RES_REQ or RES_NONE */
};

typedef struct Resource Resource;
Resource resources[MAX_RESOURCES];

enum {
    RES_HELD, RES_REQ, RES_NONE
};

/* Requests that are queued 
 * These can just be a list of packets that are waiting to be processed
 * Use push and pop fuctions to interact
 */
/* to track of the nodes that are waiting */
QueueElement queue[MAX_CV][MAX_CV_QUEUE_LEN];
MessageQueue pendingCVQueue[MAX_CV]; /* use push and pop only */
/* with great power comes a new namespace - here we have none!!*/

/* Packet types */
enum {
    EMPTY = 0x00,
    LOCK_ACQUIRE = 0x01,
    LOCK_RELEASE = 0x02,
    LOCK_OK = 0x04,
    CV_WAIT = 0x07,
    CV_SIGNAL = 0x08,
    CV_BROADCAST = 0x09,
    NODE_READY = 0x0C,
    RECP_DATA_UPDATE = 0x0D,
    PAT_DATA_UPDATE = 0x0E,
    DOORB_DATA_UPDATE = 0x0F,
    DOC_DATA_UPDATE = 0x10,
    CASH_DATA_UPDATE = 0x11,
    CLERK_DATA_UPDATE = 0x12,
    MAN_DATA_UPDATE = 0x13
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
    SENDER_ID = 0, TIMESTAMP = 2, PACKET_TYPE = 6, DATA = 7
};

/* Easy access to the data in a message */
struct packet {
    int senderId;
    int timestamp;
    char packetType;
    char data[MaxMailSize - DATA];
};
typedef struct packet Packet;

/*********************************
 ********** FUNCTIONS ************
 *********************************/

void initResources(Resource arr[]);
int addResource(Resource arr[], int id, int state);
int deleteResource(Resource arr[], int id);

int getResourceStatus(int resourceID);
int updateResourceStatus(int resourceID, int newStatus);
int getResourceReplies(int resourceID);
int updateResourceReplies(int resourceID, int replies);

/* These system calls for doing the netowork I/O
 * This also takes care of the protocol stack.
 */

int Packet_Receive(int mbox, int *senderId, int *senderMBox,
        Packet *receivedPacket);

int Packet_Send(int receiverId, int recMBox, int senderMBox, Packet*);

void SerializePacket(Packet *p, char* message);
void DeserializePacket(Packet *p, char* message);

/* Defined some methods for putting data into and out of messages */
int copyOutInt(char* message, int index);
void copyInInt(char* message, int index, int val);

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

int DistLock_Acquire(int name);
int DistLock_Release(int name);

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

/* Gets the mapping between the CV and its assiciated Lock */
int getCV_Lock_Mapping(int CVID);

/* Data update handling Functions */
int UpdateData_Patient(Packet p);
int UpdateData_Receptionist(Packet p);
int UpdateData_Doorboy(Packet p);
int UpdateData_Doctor(Packet p);
int UpdateData_Cashier(Packet p);
int UpdateData_Clerk(Packet p);
int UpdateData_HospitalManager(Packet p);

/* Hospital Entity interface for the Network Data Update */
int HDataUpdate_Recp(int id);
int HDataUpdate_Pat(int id);
int HDataUpdate_Doorb(int id);
int HDataUpdate_Doc(int id);
int HDataUpdate_Cash(int id);
int HDataUpdate_Clerk(int id);
int HDataUpdate_HospMan(int id);

int DistUpdate_Send(Packet p);
#endif /* P2PNETWORK_H */

