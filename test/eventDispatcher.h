/* This is the event Dispatcher.
 * All the messages are sent/received through this bad boy
 * This system takes care of the following:
 * 1. Total ordering
 * 2. Send messages to the network (broadcast)
 * 3. Receive messages from the network and pass it to the handler
 * During all this we use the total ordering mumbo-jumbo!
 */

/* Define a messge queue to store the messages */
/* whenever we have to send anything it has to be simply added to
 * a SendQueue and this will be looked into by the event dispatcher
 * and then it will send the message onto the network 
 */

#define MaxSendQueueSize 50
#define MaxPendingMessagesSize 100
#define MAXHOSTS 100
struct messageQueueElement{
    Packet message;
    int next; /* init = -1*/
    char valid; /* intit = 0 */
};

typedef struct messageQueueElement QueueElement;

struct MessageQueue{
    QueueElement *queue;
    int length;
    int head;
    int tail;
};

typedef struct MessageQueue MessageQueue;

/* methods for a FIFO Message Queue */

void Init_MsgQueue(MessageQueue *q);
void MsgQueue_Push(MessageQueue *q, Packet& msg);
void MsgQueue_Insert(MessageQueue *q, int pos, Packet& msg);
char[] MsgQueue_Pop(MessageQueue *q);
char MsgQueue_IsEmpty(MessageQueue *q);

/* FIFO Queue for the send messages, used by the event dispatcher */
/* This process handles the messages that are meant to be sent */
MessageQueue sendQueue[MaxSendQueueSize];
/* This process handles the messages that are received and pending processing */
MessageQueue pendingMessagesQueue[MaxPendingMessagesSize];
/* Table of the last timestamp for each host */
int LastTimestampSeen[MAXHOSTS];
/*
   1. Everytime I receive a message I update the last timestamp seen for that host.
*/
void updateLastTimestampSeen(int hostID, int timestampReceived);
/* 2. Queue up message to pending queue till further notice 
      For this we will use, void MsgQueue_Push(pendingMessagesQueue, msg);
 */