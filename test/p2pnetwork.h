#ifndef P2PNETWORK_H
#define P2PNETWORK_H

#include "syscall.h"
#include "packet.h"
#include "eventDispatcher.h"
#include "print.h"
#include "hospShared.h"

#define MAX_CV 50
#define MAX_CV_QUEUE_LEN 100
#define MAX_LOCK 50
#define MAX_LOCK_QUEUE_LEN 100
#define MaxEntities 50
#define MAX_RESOURCES 20000
#define ENTITY_OFFSET 20
#define LOCK_OFFSET 10

/************************************
 ******** GLOBAL DATA ***************
 ************************************/
int myNetThreadMbox;
int readyCount;
int numberOfEntities[7]; 
/* The number of entities
 * numberOfEntities[0] - patients
 * numberOfEntities[1] - receptionists
 * numberOfEntities[2] - doorboy
 * numberOfEntities[3] - doctors
 * numberOfEntities[4] - cashiers
 * numberOfEntities[5] - clerks
 * numberOfEntities[6] - hospital managers
 */

LockId netthread_Lock; /* We use these to interact with the netthread.*/
CVId netthread_CV; /* Used to wait on the netthread for a reply */
LockId ping_Lock; /* lock for the line below */
CVId ping_CV; /* used to wait to see if a netthred is still alive */

/* We need to keep a list of the resources that we currently hold
 * so, all the locks and CVs that we hold are with us are present in
 * this list of resources
 */

struct Resource {
    int name;
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

QueueElement lockqueue[MAX_LOCK][MAX_LOCK_QUEUE_LEN];
MessageQueue pendingLockQueue[MAX_LOCK];
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
    NODE_READY = 0x0A,
    RECP_DATA_UPDATE = 0x0B,
    PAT_DATA_UPDATE = 0x0C,
    DOORB_DATA_UPDATE = 0x0D,
    DOC_DATA_UPDATE = 0x0E,
    CASH_DATA_UPDATE = 0x0F,
    CLERK_DATA_UPDATE = 0x10,
    MAN_DATA_UPDATE = 0x11,
    GLOBAL_DATA_UPDATE = 0x12,
    DO_PING = 0x13,
    PING = 0x14,
    PONG = 0x15,
    KILL = 0x16,
    GO = 0x17
};

/* Global Variable Identification */

enum {
    NUMDOCTORS = 0x01,
    NUMCASHIERS = 0x02,
    NUMCLERKS = 0x03,
    NUMDOORBOYS = 0x04,
    NUMRECP = 0x05,
    NUMPATIENTS = 0x06,
    FEESPAID = 0x07,
    TEST_STATE = 0x08,
    TOKENCOUNTER = 0x09,
    TOTALSALES = 0x0A,
    PEOPLEINHOSPITAL = 0x0B,
    DOORBOYLINELENGTH = 0x0C,
    PATCOUNT = 0x0D,
    RECPCOUNT = 0x0E,
    DOORBCOUNT = 0x0F,
    DOCCOUNT = 0x10,
    CASHCOUNT = 0x11,
    CLERKCOUNT = 0x12,
    HOSPMANCOUNT = 0x13,
    FEELIST_APPEND = 0x14,
    QUEUE_PUSH = 0x15,
    QUEUE_POP = 0x16
};

/* Entity Identification 
enum {
    RECEPTIONIST_NODE = 0x0D,
    PATIENT_NODE = 0x0E,
    DOCTOR_NODE = 0x0F,
    DOORBOY_NODE = 0x10,
    CASHIER_NODE = 0x11,
    CLERK_NODE = 0x12,
    MANAGER_NODE = 0x13
};
*/
int myMbox;
char going;

/*********************************
 ********** FUNCTIONS ************
 *********************************/

int getMboxNum();

void initializeSystem();

void initResources();
int addResource(int id, int state, int timestamp);
int deleteResource(int id);

int getResourceStatus(int resourceID);
int getResourceTimestamp(int resourceID);
int updateResourceStatus(int resourceID, int newStatus);
int getResourceReplies(int resourceID);
int updateResourceReplies(int resourceID, int replies);

void SendAll(int packetType);

/* tell everyone to exchange packets */
void HMultiPing();

/* end all network threads */
void HKill();
void Kill();

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
 * distributed system NODEPS
 */
int HCV_Signal(int HCVId, int HLockId);
int HCV_Wait(int HCVId, int HLockId);
int HCV_Broadcast(int HCVId, int HLockId);

int DistCV_Wait(int CVID, int LockID);
int DistCV_Signal(int CVID);
void Process_CV_Signal(Packet pkt);

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

void readConfig();

/* Gets the mapping between the CV and its assiciated Lock */
int getCV_Lock_Mapping(int CVID);

/* Create Node Ready */
void HNodeReady();

/* Hospital Entity interface for the Network Data Update NO DEPS*/
int HDataUpdate_Recp(int id, int peopleInLine, int currentToken);
int HDataUpdate_Pat(int id);
int HDataUpdate_Doorb(int id);
int HDataUpdate_Doc(int id, int peopleInLine, int prescription,
        int patientToken);
int HDataUpdate_Cash(int id, int lineLength, int patToken, int fee,
        int payment, int sales);
int HDataUpdate_Clerk(int id, int patientsInLine, int payment, int fee,
        int patPrescription, int sales);
int HDataUpdate_HospMan(int id);

/* Ping my network Thread for life! */
void HPing_NetThread();

/* Updates the Global data variables NO DEPS*/
int HGlobalDataUpdate(short Variable, int val);
int HGlobalListAppendUpdate(int key, int value);
int HGlobalQueuePopUpdate();
int HGlobalQueuePushUpdate(int value);
int DistUpdate_Send(Packet p);
int DistGlobalData_Send(Packet p);

#endif /* P2PNETWORK_H */

