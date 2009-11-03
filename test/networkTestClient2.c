#include "print.c"
#include "syscall.h"

void main(){
    /* This client sends message "Hello Client 1!" to
     * machine Id 1 with mailbox number 0
     * This client then waits for a message from machine Id
     * 1 on mailbox 0 and then displays it
    */

    int response;
    char *message = "bar";
    char *receivedMessage;

    response = Receive(0, receivedMessage);
    print("Client 1: ");
    print(receivedMessage);
    print("\nClient 1: Sending bar to client 0/0\n");
    Send(0, 0, message);
    Exit(0);
}
