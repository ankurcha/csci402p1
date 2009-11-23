#include "init.h"

void clerk(int ID) {
    char str[50];
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
        clerks[ID].sales += clerks[ID].payment;
        /** TODO: PUSH DATA TO NETWORK **/
        HLock_Release(PaymentLock);
        HLock_Release(clerks[ID].ClerkTransLock);
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
    /*Initialize datastructures for all the threads
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

    /*Spawning Pharmacys clerks */
    numClerks = numberOfEntities[5];
    print("Creating ");
    print(itoa(numClerks, str));
    print(" Clerks\n");
    for (i = 0; i < numClerks; i++)
        Fork(createPharmacyClerk);

    for (i = 0; i < 100; i++)
        Yield();
}
