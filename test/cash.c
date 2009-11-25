#include "init.h"
#include "netthread.c"

/*
 * We need some sort of data update mechanism so that
 * everyone knows when we update values.
 */

void createCashier() {
    int temp;
    char str[50];
    initializeSystem();
    print("Forking network_thread...");
    Fork(network_thread);
    print("done\n");
    print("Forking cashier...");
    print(itoa(patientCount, str));
    print("\n");
/*    
    HLock_Acquire(creationLock);
    temp = cashierCount;
    cashierCount++;
    HLock_Release(creationLock);
*/
    cashier(myMbox);
    print("done\n");
    Exit(0);
}

void cashier(int ID) {
    char str[50];
    HNodeReady();
    print("Cash_");
    print(itoa(ID, str));
    print(":  Alive!!\n");
    while (1) {
        HLock_Acquire(cashierLineLock);
        if (cashiers[ID].lineLength > 0) { /* someone in line */
            /*signal person on top */
            print("Cash_");
            print(itoa(ID, str));
            print(":  someone in my line...\n");
            HCV_Signal(cashiers[ID].lineCV, cashierLineLock);
        } else { /* noone in line */
            /* go on break */
            /* prefix for test condition */
            if (test_state == 11)
                print("T11: ");
            print("Cash_");
            print(itoa(ID, str));
            print(":  No one in line... going on break\n");
            HCV_Wait(cashiers[ID].breakCV, cashierLineLock);
            Release(cashierLineLock);
            continue;
        }

        /* I have a patient */
        /* acquire transLock and use it to govern transactions */
        /*  with the patient */
        HLock_Acquire(cashiers[ID].transLock);
        HLock_Release(cashierLineLock);

        /* waiting for patient to deposit its token in patToken */
        HCV_Wait(cashiers[ID].transCV, cashiers[ID].transLock);

        /* lookup value for cashiers[ID].patToken in the token table */
        HLock_Acquire(feeListLock);
        cashiers[ID].fee = List_getValue(&feeList, cashiers[ID].patToken);
        HDataUpdate_Cash(ID, cashiers[ID].lineLength, cashiers[ID].patToken,
                cashiers[ID].fee, cashiers[ID].payment, cashiers[ID].sales);
        HLock_Release(feeListLock);
        /* tell patient the fee */
        HCV_Signal(cashiers[ID].transCV, cashiers[ID].transLock);
        /* wait for payment */
        HCV_Signal(cashiers[ID].transCV, cashiers[ID].transLock);
        /* add this payment to our total collected */
        HLock_Acquire(feesPaidLock);
        feesPaid += cashiers[ID].payment;
        cashiers[ID].sales += cashiers[ID].payment;
        HDataUpdate_Cash(ID, cashiers[ID].lineLength, cashiers[ID].patToken,
                cashiers[ID].fee, cashiers[ID].payment, cashiers[ID].sales);
        HLock_Release(feesPaidLock);
        if (cashiers[ID].payment < cashiers[ID].fee)
            print("ERROR: call security, that patient didin't pay!");
        HLock_Release(cashiers[ID].transLock);
    }
    Exit(0);
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
    /*Initialize data structures for all the threads
     //1. Patients don't need initialization
     //2. Receptionists
     */
    for (i = 0; i < RECP_MAX; i++) {
        __Receptionists(&receptionists[i], i);
    }
    for (i = 0; i < MAX_DOCTORS; i++) {
        __Doctor(&doctors[i], i);
    }
    for (i = 0; i < MAX_CASHIER; i++) {
        __Cashier(&cashiers[i], i);
    }
    for (i = 0; i < MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i], i);
    }

    readConfig();
    /* spawn the cashier Threads */
    numCashiers = numberOfEntities[4];
    createCashier();
    Exit(0);
}
