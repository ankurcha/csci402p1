/* Main function to execute the distributed hospital
 *  This function will spawn the required number of processes
 *  of the entity type specified by the machine id
 *
 * USC CS 402 Fall 2009
 * Group 11
 * Max Pflueger
 */

#include "p2pnetwork.h"

int main() {
    int machineID = 0;
    int threadCt = 0;
    int i = 0;

    machineID = GetMachineID();

    /*TODO determine threadCt by reading conf file */

    /*TODO put msgs into mailbox 0 with mail box numbers */

    switch (machineID) {
        case 0: /* patients */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/patient");
            }
            break;
        case 1: /* receptionists */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/receptionist");
            }
            break;
        case 2: /* doorboys */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/doorboy");
            }
            break;
        case 3: /* doctors */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/doctor");
            }
            break;
        case 4: /* cashiers */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/cashier");
            }
            break;
        case 5: /* clerks */
            for (i = 0; i < threadCt; i++) {
                Exec("../test/clerk");
            }
            break;
        case 6: /* hospital manager */
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

