#include "print.c"
#include "syscall.h"
 
int main(){
    /* This client sends message "Hello Client 2!" to
* machine Id 1 with mailbox number 0
* This client then waits for a message from machine Id
* 1 on mailbox 0 and then displays it
*/
 
  int response;
  char *message = "foo";
  char *receivedMessage;
  print("Client 1: Sending foo\n");
  Send(1, 0, message);
 
  response = Receive( 0, receivedMessage);
 
  print("Client 1: ");
  print(receivedMessage);
  print("\n");
  print("Trying to send to a bad receiver = -1\n");
  Send(-1,0, message);
  print("\n Trying to receive from bad receiver = -1\n");
  response = Receive(-1, receivedMessage);
  print("Client 1: ");
  print(receivedMessage);
  print("\n");
 
  Exit(0);
}
