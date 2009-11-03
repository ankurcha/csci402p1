#include "print.c"
#include "syscall.h"
#include "itoa.c"
#define MACHINENUM 2

int main(){
    /* This client sends message "Hello Client 2!" to
     * machine Id 1 with mailbox number 0
     * This client then waits for a message from machine Id
     * 1 on mailbox 0 and then displays it
    */

    int response;
    char *message = "foo  ";
    char *receivedMessage;
    int i, netname;
    char NetID[5];
    char a[5];
    int destinationId[5] = {
        0,1,2,3,4
    };
    /* create a custom message */
    netname = GetMachineID();
    itoa(netname, NetID);
    message[4] = NetID[0];
    for (i=0; i<MACHINENUM; i++) {
        if(i == GetMachineID())
            continue; 
        print("Client ");
        print(NetID);
        print(": Sending foo to machine# ");
        print(itoa(i,a));
        print(" mailbox# 0\n");
        Send(i, 0, message);
    }
    
    print("Printing Received Messages\n");
    for (i=0; i<MACHINENUM; i++) {
        if(i == GetMachineID())
            continue;
        print("Messages from machine#");
        print(itoa(i,a));
        print("\n");
        while((response = Receive(i, 0, receivedMessage))<=0);
        
        print("Client ");
        print(NetID);
        print(": ");
        print(receivedMessage);
        print("\n");
    }
    
    /*print("Trying to send to a bad receiver = -1\n");
    Send(-1,0, message);
    
    print("\n Trying to receive from bad receiver = -1\n");
    response = Receive(-1, 0, receivedMessage);
    print("Client ");
    print(NetID);
    print(": Received ");
    print(receivedMessage);
    print("\n");
    */
    Exit(0);
}
