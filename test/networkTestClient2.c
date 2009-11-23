#include "print.c"
#include "syscall.h"

int main() {
	/* This client sends message "Hello Client 1!" to
	 * machine Id 1 with mailbox number 0
	 * This client then waits for a message from machine Id
	 * 1 on mailbox 0 and then displays it
	 */

        int senderId, senderMbox;
	int response;
	char *message = "bar";
	char *receivedMessage;

	response = Receive(0, &senderId, &senderMbox, receivedMessage);
	print("Client 2: ");
	print(receivedMessage);
	print("\nClient 2: Sending bar\n");
	Send(0, 0, 0, message);
	print("Trying to send to a bad receiver = -1\n");
	Send(-1, 0, 0, message);
	print("\n Trying to receive from bad receiver = -1\n");
	response = Receive(-1, &senderId, &senderMbox, receivedMessage);
	print("Client 1: ");
	print(receivedMessage);
	print("\n");
	Exit(0);
}
