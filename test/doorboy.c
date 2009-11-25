#include "init.h"
#include "netthread.c"

void createDoorBoy() {
    int temp;
    char str[50];
    initializeSystem();
    print("Forking network_thread...");
    Fork(network_thread);
    print("done\n");
    print("Forking Doorboy...");
    print(itoa(patientCount, str));
    print("\n");
/*
    HLock_Acquire(creationLock);
    temp = doorboyCount;
    doorboyCount++;
    HLock_Release(creationLock);
*/
    doorboy(myMbox);
    print("done\n");
    Exit(0);
}

void doorboy(int ID) {
    char str[50];
    int myDoctor = 0;
    char doorboyBreak = 0;
    HNodeReady();
    while (1) {

        print("DB_");
        print(itoa(ID, str));
        print(": Alive ");

        /*Get into the doorboyLine till some doctor asks for me */
        HLock_Acquire(doorboyLineLock);
        doorboyLineLength++;
        HGlobalDataUpdate(DOORBOYLINELENGTH, doorboyLineLength);
        print("DB_");
        print(itoa(ID, str));
        print(": Waiting for some doctor to wake me up.");
        print("\n");
        HCV_Wait(doorboyLineCV, doorboyLineLock);
        doorboyLineLength--;
        HGlobalDataUpdate(DOORBOYLINELENGTH, doorboyLineLength);
        /*Some doctor woke me up, lets check who */
        /*myDoctor =  wakingDoctorID; */
        if (Queue_IsEmpty(&wakingDoctorList) == 1) {
            print("DB_");
            print(itoa(ID, str));
            print(": ERROR: Waking doctor list is empty!\n");
            continue;
        }
        myDoctor = Queue_Pop(&wakingDoctorList);
        HGlobalQueuePopUpdate();
        if (test_state == 2) {
            print("DB_");
            print(itoa(ID, str));
            print(":TEST2: Servicing D_");
            print(itoa(myDoctor, str));
            print("\n");
        } else {
            print("DB_");
            print(itoa(ID, str));
            print(":Servicing D_");
            print(itoa(myDoctor, str));
            print("\n");
        }
        HLock_Release(doorboyLineLock);
        /* Inform the doctor that I have arrived, and wait for him to take  */
        /*  a break, if he so chooses */
        HLock_Acquire(doctors[myDoctor].transLock);
        HCV_Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
        HCV_Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);

        /*/// PATIENT LINE ///// */
        /*Acquire the lock to get the state of the line and take decision */
        HLock_Acquire(doctors[myDoctor].LineLock);
        print("DB_");
        print(itoa(ID, str));
        print(": Checking for Patients\n");

        /*while there is noone in line */
        doorboyBreak = 0;
        while (doctors[myDoctor].peopleInLine <= 0) {
            doorboyBreak = 1;
            /*I will be woken up by the manager only!! */
            /* prefix for test conditions */
            if (myDoctor == 0 && test_state == 8)
                print("T8: ");
            if (test_state == 11)
                print("T11: ");
            if (test_state == 2) {
                print("DB_");
                print(itoa(ID, str));
                print(":TEST2: Yawn!!...ZZZZzzzzz....\n");
            } else {
                print("DB_");
                print(itoa(ID, str));
                print(": Yawn!!...ZZZZzzzzz....\n");
            }
            HCV_Wait(doctors[myDoctor].doorboyBreakCV,
                    doctors[myDoctor].LineLock);
            /* I got woken up, time to go back to work - by now there are  */
            /*  people dying on the floor! */
        }
        if (doorboyBreak) {
            /* prefix for test 8 condition */
            if (myDoctor == 0 && test_state == 8) {
                print("T8: \n");
            }
            print("DB_");
            print(itoa(ID, str));
            print(": Woken up!\n");
        }
        print("DB_");
        print(itoa(ID, str));
        print(": Found ");
        print(itoa(doctors[myDoctor].peopleInLine, str));
        print(" patients waiting in line for D_ ");
        print(itoa(myDoctor, str));
        print("\n");
        /*Now wake the patient up to go to the doctor */
        print("DB_");
        print(itoa(ID, str));
        print(":Tell patient to go to doctor D_");
        print(itoa(myDoctor, str));
        print("\n");
        HCV_Signal(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);
        /*My job with the patients and the doctor is done */
        /*I can go back on the doorboyLine */
        Release(doctors[myDoctor].transLock);
        Release(doctors[myDoctor].LineLock);
    }/*End of while */
    print("DB_");
    print(itoa(ID, str));
    print(":Dying...AAAaaaahhhhhhhhh!!\n");
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
    numDoctors = numberOfEntities[2];
    numDoorboys = numDoctors;
    createDoorBoy();
    Exit(0);
}

