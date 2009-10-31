#include "print.c"
#include "syscall.h"

int main(){
    /* This client sends message "Hello Client 1!" to
     * machine Id 1 with mailbox number 0
     * This client then waits for a message from machine Id
     * 1 on mailbox 0 and then displays it
    */

    int response;
    char *message = "bar";
    char *receivedMessage;

    response = Receive(0, 0, receivedMessage);
    print("Client 2: ");
    print(receivedMessage);
    print("\nClient 2: Sending bar\n");
    Send(0, 0, message);
    Exit(0);
}
