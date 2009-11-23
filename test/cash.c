#include "init.h"
/*
 * We need some sort of data update mechanism so that
 * everyone knows when we update values.
 */
void cashier(int ID) {
    char str[50];
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
            HLock_Signal(cashiers[ID].lineCV, cashierLineLock);
        } else { /* noone in line */
            /* go on break */
            /* prefix for test condition */
            if (test_state == 11)
                print("T11: ");
            print("Cash_");
            print(itoa(ID, str));
            print(":  No one in line... going on break\n");
            HLock_Wait(cashiers[ID].breakCV, cashierLineLock);
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
        HDataUpdate_Cash(ID);
        HLock_Release(feeListLock);
        /* tell patient the fee */
        HCV_Signal(cashiers[ID].transCV, cashiers[ID].transLock);
        /* wait for payment */
        HCV_Signal(cashiers[ID].transCV, cashiers[ID].transLock);
        /* add this payment to our total collected */
        HLock_Acquire(feesPaidLock);
        feesPaid += cashiers[ID].payment;
        cashiers[ID].sales += cashiers[ID].payment;
        HDataUpdate_Cash(ID);
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
    /*Initialize data structures for all the threads
     //1. Patients don't need initialization
     //2. Receptionists
     */
    Write("Initializing Recptionists DS\n", 25, 1);
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

    /* spawn the cashier Threads */
    numCashiers = numberOfEntities[4];

    print("Creating ");
    print(itoa(numCashiers, str));
    print(" Cashiers\n");

    for (i = 0; i < numCashiers; i++)
        Fork(createCashier);

    /*HospINIT(testmode);*/
    for (i = 0; i < 100; i++)
        Yield();
}
