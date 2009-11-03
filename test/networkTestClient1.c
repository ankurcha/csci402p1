#include "print.c"
#include "syscall.h"

void main(){
    /* This client sends message "Hello Client 2!" to
     * machine Id 1 with mailbox number 0
     * This client then waits for a message from machine Id
     * 1 on mailbox 0 and then displays it
    */

  int response = 0;
  char *message = "foo";
  char *receivedMessage = "---";
  print("Client 0: Sending foo to client 1/0\n");
  
  Send(1, 0, message); 
  response = Receive(0, receivedMessage);
  print("Client 0: ");
  print(receivedMessage);
  print("\n");
    print("Shit\n");
  Exit(0);
}
