#inlcude "syscall.h"
#define MaxMailSize 40
#define MaxDataSize (MaxMailSize - 4)
/*
 * These system calls for doing the netowork I/Oi
 * This also takes care of the protocol stack.
 *
 */

int Hospital_Receive(int, int, Packet&);

int Hospital_Send(int receiverId, int mailboxId, Packet&);

int Hospital_Multicast(int[], int[], int, Packet&);

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
 * data[2] and data[3] - Entity Id (sized int hence 2 bytes)
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
    NODE_READY = 0x0C
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

struct packet{
    int senderId;
    int timestamp;
    char *data;
};

typedef struct packet Packet;
char *SerializePacket(Packet&, char*, int senderId);
int DeserializePacket(Packet&, char*, int senderId);

/* Register/Acquire/Release/Destroy a lock with the distributed system 
 * causes all to become aware that this lock is available for messing with
 */
int HLock_Create(int HlockId, int EntityID);
int HLock_Acquire(int HlockId, int EntityID);
int HLock_Release(int HlockId, int EntityID);
int HLock_Destroy(int HlockId, int EntityID);

/* Register/Signal/Wait/Broadcast/Destroy a Condition variable with the
 * distributed system
 */
int HCV_Signal(int HCVId, int HLockId, int EntityID);
int HCV_Wait(int HCVId, int HLockId, int EntityID);
int HCV_Broadcast(int HCVId, int HLockId, int EntityID);

/* A synchronized print routine to print the lines on the console nicely
 */
int SyncPrint(char *, int EntityID);
