#include "print.c"
#include "syscall.h"

int main(){
    /* This client sends message "Hello Client 2!" to
     * machine Id 1 with mailbox number 0
     * This client then waits for a message from machine Id
     * 1 on mailbox 0 and then displays it
    */

    int response;
    char message[16] = "Hello Client 2!";
    char receivedMessage[16];

    Send(1, 0, message); 

    response = Receive(0, 0, receivedMessage);

    print("Client 1: Message Received: ");
    print(receivedMessage);

    Exit(0);
}
