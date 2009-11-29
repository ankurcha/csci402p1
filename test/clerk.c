#include "init.h"
#include "netthread.c"

void createPharmacyClerk() {
    int temp;
    unsigned char str[50];
    initializeSystem();
    print("Forking network_thread...");
    Fork(network_thread);
    print("done\n");
    print("Forking clerk...");
/*
    HLock_Acquire(creationLock);
    temp = pharmacyCount;
    pharmacyCount++;
    HLock_Release(creationLock);
*/
    clerk(myMbox);
    print("done\n");
}

void clerk(int ID) {
    unsigned char str[50];
    HNodeReady();
    print("Clerk Alive!!\n");
    while (1) {
        HLock_Acquire(ClerkLinesLock);
        if (clerks[ID].patientsInLine > 0) {
            /* someone in line */
            /*signal the first person */
            HCV_Signal(clerks[ID].ClerkCV, ClerkLinesLock);
        } else { /* noone in line */
            /* go on break */
            /* prefix for test condition */
            if (test_state == 11)
                print("T11: ");
            print("CL_");
            print(itoa(ID, str));
            print(": Going on break\n");
            HCV_Wait(clerks[ID].ClerkBreakCV, ClerkLinesLock);
            HLock_Release(ClerkLinesLock);
            continue;
        }

        /* I have a patient */
        /* acquire the transaction Lock for further transactions */
        /*  with the patient */
        HLock_Acquire(clerks[ID].ClerkTransLock);
        HLock_Release(ClerkLinesLock);
        /* waiting for patient to give prescription */
        HCV_Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
        /* patient gives prescription: */
        print("CL_");
        print(itoa(ID, str));
        print(": gave Medicines!\n");
        HCV_Signal(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
        /* wait for payment */
        HCV_Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
        /*Collect payment */
        print("CL_");
        print(itoa(ID, str));
        print(": The cost for the medicines are:");
        print(itoa(clerks[ID].fee, str));
        print(" Dollars\n");
        if (clerks[ID].payment != clerks[ID].fee)
            print("ERROR: patient did not pay for medicine\n");
        /* add this payment to our total collected */
        HLock_Acquire(PaymentLock);
        totalsales += clerks[ID].payment;
        HGlobalDataUpdate(TOTALSALES, totalsales);
        clerks[ID].sales += clerks[ID].payment;
        HDataUpdate_Clerk(ID, clerks[ID].patientsInLine, clerks[ID].payment,
                clerks[ID].fee, clerks[ID].patPrescription, clerks[ID].sales);
        HLock_Release(PaymentLock);
        HLock_Release(clerks[ID].ClerkTransLock);
    }
    Exit(0);
}

int main(int argc, char** argv) {
    int i;
    unsigned char inp[20];
    unsigned char str[50];
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
    for (i = 0; i < MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i], i);
    }

    /*Spawning Pharmacys clerks */
    readConfig();
    numClerks = numberOfEntities[5];
    createPharmacyClerk();
    Exit(0);
}
