#include "print.c"
#include "syscall.h"
#include "itoa.c"
#define MACHINENUM 2

int main() {
    int response = 0, ackResp = 0, t = 0;
    char *message;
    char *receivedMessage;
    int i, netname;
    char NetID[32];
    char a[5];
    int destinationId[5] = { 0, 1, 2, 3, 4 };
    /* create a custom message */
    netname = GetMachineID();
    NetID = itoa(netname, NetID);
    message = "SYN";

    /*
     Send messages to others.
     */
    print("HelloWorld\n");
    for (i = 0; i < 2; i++) {
        print("Client ");
        print(NetID);
        print(": Sending ");
        print(message);
        print(" to machine# ");
        print(itoa(i, a));
        print(" mailbox# 0\n");
        Send(i, 0, 0, message);
    }

    print("Printing Received Messages\n");
    for (i = 0; i < 2; i++) {
        print("Messages from machine#");
        print(itoa(i, a));
        print("\n");
        response = Receive(i, 0, 0, receivedMessage);
        print("Client ");
        print(NetID);
        print(": ");
        print(receivedMessage);
        print("\n");
    }
    Exit(0);
}
