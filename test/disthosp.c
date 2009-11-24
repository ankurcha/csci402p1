/* Main function to execute the distributed hospital
 *  This function will spawn the required number of processes
 *  of the entity type specified by the machine id
 *
 * USC CS 402 Fall 2009
 * Group 11
 * Max Pflueger
 */

#include "syscall.h"
#include "p2pnetwork.h"
#include "packet.h"

int main() {
    int machineID = 0;
    int threadCt = 0;
    int i = 0;
    char buf[50];
    char message[MaxMailSize];
    machineID = GetMachineID();
    /* determine threadCt by reading conf file */
    readConfig();
    threadCt = numberOfEntities[machineID];

    /* put msgs into mailbox 0 with mail box numbers */
    for(i=0; i < threadCt; i++) {
        copyInInt(message, 0, i+1);
        Send(machineID, 0, 0, message);
    }

    switch (machineID) {
        case 0: /* patients */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" patient entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/patient");
            }
            break;
        case 1: /* receptionists */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" receptionist entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/receptionist");
            }
            break;
        case 2: /* doorboys */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" doorboy entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/doorboy");
            }
            break;
        case 3: /* doctors */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" doctor entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/doctor");
            }
            break;
        case 4: /* cashiers */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" cashier entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/cash");
            }
            break;
        case 5: /* clerks */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" clerk entities.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/clerk");
            }
            break;
        case 6: /* hospital manager */
            print("disthosp: preparing to create ");
            print(itoa(threadCt, buf));
            print(" hospManager entity.\n");
            for (i = 0; i < threadCt; i++) {
                Exec("../test/hospManager");
            }
            break;
        default:
            print("ERROR: specified invalid machine number\n");
            return -1;
    }

    return 0;
}

