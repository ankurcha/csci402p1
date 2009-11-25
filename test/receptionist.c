/*
 *  receptionist.c
 *  
 *
 *  Created by Ankur Chauhan on 11/21/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */
#include "init.h"
#include "netthread.c"
void createReceptionist() {
    int temp;
    char str[50];
    initializeSystem();
    print("Forking network_thread...");
    Fork(network_thread);
    print("done\n");
    print("Forking receptionist# ");
    print(itoa(myMbox, str));
    print("...");
/*
    HLock_Acquire(creationLock);
    temp = recptionistCount;
    recptionistCount++;
    HLock_Release(creationLock);
*/    
    receptionist(myMbox);
    print("done\n");
    Exit(0);
}

void receptionist(int ID) {
    char str[50];
    HNodeReady();
    while (1) {

        print("R_");
        print(itoa(ID, str));
        print(": Alive!\n");

        HLock_Acquire(recpLineLock);

        if (receptionists[ID].peopleInLine > 0) {
            /*Wake one waiting patient up */
            HCV_Signal(receptionists[ID].receptionCV, recpLineLock);
        } else {
            /*My Line is empty */
            /* prefix for test condition */
            if (test_state == 11)
                print("T11: ");
            print("R_");
            print(itoa(ID, str));
            print(":Going to sleep\n");
            HCV_Wait(receptionists[ID].ReceptionistBreakCV, recpLineLock);
            HLock_Release(recpLineLock);
            /*HospitalManager kicked my ass for sleeping on the job!! */
            /*Loop back!! */
            continue;
        }
        HLock_Acquire(receptionists[ID].transLock);
        HLock_Release(recpLineLock);
        /*Genetate token for the patient */
        HLock_Acquire(TokenCounterLock);
        print("R_");
        itoa(ID, str);
        print(str);
        print(": Generating Token...\n");

        receptionists[ID].currentToken = ++TokenCounter;
        HGlobalDataUpdate(TOKENCOUNTER, TokenCounter);
        HDataUpdate_Recp(ID, receptionists[ID].peopleInLine,
                receptionists[ID].currentToken);

        HLock_Release(TokenCounterLock);

        /*Sleep till you get Acknowledgement */

        print("R_");
        print(itoa(ID, str));
        print(":  Waiting for Patient to pick up token...\n");

        HCV_Wait(receptionists[ID].receptionistWaitCV,
                receptionists[ID].transLock);

        /*Patient successfully got the token, go back to work: Loop again */
        print("R_");
        print(itoa(ID, str));
        print(": Patient got token, Continue to next Patient\n");
        HLock_Release(receptionists[ID].transLock);
    }
    Yield();
}

int main(int argc, char** argv) {
    int i;
    char inp[20];
    char str[50];
    testlock = 10000;
    TokenCounterLock = 10001;
    recpLineLock = 10002;
    feeListLock = 10003;
    cashierLineLock = 10004;
    feesPaidLock = 10005;
    ClerkLinesLock = 10006;
    PaymentLock = 10007;
    hospitalLock = 10008;
    doorboyLineLock = 10008;
    doorboyLineCV = 10009;
    creationLock = 10010;
    wakingDoctorList.queue = wakingdoctor_element;
    wakingDoctorList.length = MAX_PATIENTS;
    wakingDoctorList.head = -1;
    wakingDoctorList.tail = -1;
    Init_Queue(&wakingDoctorList);
    feeList.head = 0;
    /*Initialize datastructures for all the threads
     //1. Patients don't need initialization
     //2. Receptionists
     */
    for (i = 0; i < RECP_MAX; i++) {
        __Receptionists(&receptionists[i], i);
    }
    /*3. DoorBoy doesn't need anything
     4. Doctors*/
    for (i = 0; i < MAX_DOCTORS; i++) {
        __Doctor(&doctors[i], i);
    }
    /*5. Cashiers*/
    for (i = 0; i < MAX_CASHIER; i++) {
        __Cashier(&cashiers[i], i);
    }
    /*6. Clerks */
    for (i = 0; i < MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i], i);
    }
    readConfig();
    numRecp = numberOfEntities[1];
    Fork(createReceptionist);
    Exit(0);
}

