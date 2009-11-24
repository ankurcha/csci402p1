/*
 *  receptionist.c
 *  
 *
 *  Created by Ankur Chauhan on 11/21/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

#include "init.h"

void receptionist(int ID) {
    char str[50];
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
        HDataUpdate_Recp(ID, receptionists[ID].peopleInLine, receptionists[ID].currentToken);

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
    strcpy(testlock, "TestLock");
    strcpy(TokenCounterLock, "TokenCounterLock");
    strcpy(recpLineLock, "recpLineLock");
    strcpy(feeListLock, "feeListLock");
    strcpy(cashierLineLock, "cashierLineLock");
    strcpy(feesPaidLock, "feesPaidLock");
    strcpy(ClerkLinesLock, "ClerkLineLock");
    strcpy(PaymentLock, "PaymentLock");
    strcpy(hospitalLock, "HospitalLock");
    strcpy(doorboyLineLock, "doorboyLineLock");
    strcpy(doorboyLineCV, "doorboyLineCV");
    strcpy(creationLock, "creationLock");
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
    print("Initializing Recptionists DS\n");
    for (i = 0; i < RECP_MAX; i++) {
        __Receptionists(&receptionists[i], i);
    }
    /*3. DoorBoy doesn't need anything
     4. Doctors*/
    print("Initializing Doctors DS\n");
    for (i = 0; i < MAX_DOCTORS; i++) {
        __Doctor(&doctors[i], i);
    }
    print("Initializing Cashiers DS\n");
    /*5. Cashiers*/
    for (i = 0; i < MAX_CASHIER; i++) {
        __Cashier(&cashiers[i], i);
    }
    print("Initializing Clerks DS\n");
    /*6. Clerks */
    for (i = 0; i < MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i], i);
    }
    /* 7. Hospital Manager */
    print("Initializing Hospital Manager DS\n");
    for (i = 0; i < totalHospMan; i++) {

    }

    numRecp = numberOfEntities[1];

    print("Creating ");
    print(itoa(numRecp, str));
    print(" Receptionists\n");

    for (i = 0; i < numRecp; i++)
        Fork(createReceptionist);

    for (i = 0; i < 100; i++)
        Yield();
}

