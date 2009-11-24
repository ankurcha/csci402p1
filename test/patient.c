/*    Hospital Management Simulation */
/* */
/*    Patient Thread Function */
/*  */
/* CS 402 Fall 2009 */
/* Group 11 */
/*  Ankur Chauhan, ankurcha */
/*  Max Pflueger, pflueger */
/*  Aneesha Mathew, aneesham */
#include "init.h"

void createPatient() {
    int temp;
    Acquire(creationLock);
    temp = patientCount;
    patientCount++;
    Release(creationLock);
    patients(temp);
    Exit(0);
}

void patients(int ID) {
    char str[50];
    /* declare variables */
    int myToken;
    int myDoctor;
    int myPrescription;
    int shortestline = 0;
    int len = 0;
    int i = 0;
    int myCashier = 0;
    int sLen = 0;
    int shortestclerkline = 0;
    int length = 0;
    len = receptionists[0].peopleInLine;
    /*//////////////////////////////////////////////// */
    /*//// Begin interaction with Receptionist /////// */
    /*//////////////////////////////////////////////// */
    print("P:Attempt to acquire recpLineLock...\n");
    HLock_Acquire(recpLineLock);
    print("P:success\n");
    /*Find shortest Line */
    if (test_state == 4) {
        print(
                "P:TEST4: Searching for the receptionist with the shortest line\n");
    } else {
        print("P: Searching for the receptionist with the shortest line\n");
    }
    for (i = 0; i < numRecp; i++) {
        if (test_state == 4) {
            /*Print the length of each line */
            print("P:TEST4: Length of line at R_");
            print(itoa(i, str));
            print(" = ");
            print(itoa(receptionists[i].peopleInLine, str));
            print("\n");
        }
        if (receptionists[i].peopleInLine < len) {
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    if (test_state == 4) {
        print("P_");
        print(itoa(ID, str));
        print(":TEST4: Found shortest line with R_");
        print(itoa(shortestline, str));
        print(" len: ");
        print(itoa(len, str));
        print("\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(": Found shortest line with R_");
        print(itoa(shortestline, str));
        print(" len: ");
        print(itoa(len, str));
        print("\n");
    }
    /* Wait in line */
    receptionists[shortestline].peopleInLine++;
    HDataUpdate_Recp(shortestline, receptionists[shortestline].peopleInLine,
            receptionists[shortestline].currentToken);
    HCV_Wait(receptionists[shortestline].receptionCV, (recpLineLock));
    print("P_");
    print(itoa(ID, str));
    print(" Got woken up, get out of line and going to counter for token\n");
    receptionists[shortestline].peopleInLine--;
    HDataUpdate_Recp(shortestline, receptionists[shortestline].peopleInLine,
            receptionists[shortestline].currentToken);
    /*wait for the receptionist to prepare token for me, till then I wait */
    HLock_Release(recpLineLock);
    HLock_Acquire(receptionists[shortestline].transLock);
    /*token is ready just read it -- print it out in our case */
    print("P_");
    print(itoa(ID, str));
    print(": Reading Token..\n");
    myToken = receptionists[shortestline].currentToken;
    print("P_");
    print(itoa(ID, str));
    print(": My token is ");
    print(itoa(ID, str));
    print("..yeah!!\n");
    /*Done, signal receptionist that he can proceed  */
    HCV_Signal(receptionists[shortestline].receptionistWaitCV,
            receptionists[shortestline].transLock);
    print("P_");
    print(itoa(ID, str));
    print(": Signal receptionist R_");
    print(itoa(shortestline, str));
    print(" to continue, I am done\n");
    /*Release transaction lock */
    HLock_Release(receptionists[shortestline].transLock);

    /*///////////////////////////////////////////////// */
    /*///// Interaction with Doctor and Doorboy /////// */
    /*///////////////////////////////////////////////// */

    /*Calculate which doctor I want to see */
    myDoctor = (int) (Random()) % numDoctors;
    if (test_state == 2) {
        print("P_");
        print(itoa(ID, str));
        print(" :TEST2: Going to meet doctor D_");
        print(itoa(myDoctor, str));
        print("\n");
    }
    if (test_state == 7) {
        print("P_");
        print(itoa(ID, str));
        print(" :TEST7: Waiting in doctor D_");
        print(itoa(myDoctor, str));
        print(" Queue\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(" : Going to meet doctor D_\n");
    }
    /* Acquire doc's line lock */
    HLock_Acquire(doctors[myDoctor].LineLock);
    /* Wait on the line -- to be woken up by the doorboy */
    if (test_state == 2) {
        print("P_");
        print(itoa(ID, str));
        print(" :TEST2: Join line and Waiting for doorboy to tell me to go\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(" : Join line and Waiting for doorboy to tell me to go\n");
    }
    doctors[myDoctor].peopleInLine++;
    HDataUpdate_Doc(myDoctor, doctors[myDoctor].peopleInLine,
            doctors[myDoctor].prescription, doctors[myDoctor].patientToken);
    HCV_Wait(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);
    print("P_");
    print(itoa(ID, str));
    print(" : Doorboy told me to go to doctor, proceeding....\n");
    doctors[myDoctor].peopleInLine--;
    HDataUpdate_Doc(myDoctor, doctors[myDoctor].peopleInLine,
            doctors[myDoctor].prescription, doctors[myDoctor].patientToken);
    /* move into the doctor's transaction lock */
    HLock_Release(doctors[myDoctor].LineLock);
    print("P_");
    print(itoa(ID, str));
    print(" : Trying to acquire doctor's translock\n");

    HLock_Acquire(doctors[myDoctor].transLock);

    print("P_");
    print(itoa(ID, str));
    print(" : Success\n");

    print("P_");
    print(itoa(ID, str));
    print(" : Giving doctor the token...\n");

    /*The doctor is waiting for me to provide my info, oblige him!! */
    doctors[myDoctor].patientToken = myToken;
    HDataUpdate_Doc(myDoctor, doctors[myDoctor].peopleInLine,
            doctors[myDoctor].prescription, doctors[myDoctor].patientToken);
    /* hand off to the doctor thread for consultation */
    if (test_state == 7) {
        print("P_");
        print(itoa(ID, str));
        print(" :TEST7: Consulting Doctor D_");
        print(itoa(myDoctor, str));
        print(" now...\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(" : Consulting Doctor D_");
        print(itoa(myDoctor, str));
        print(" now...\n");
    }

    HCV_Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    HCV_Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);

    /* Consultation finished, now I have to get the prescription from the doctor */
    myPrescription = doctors[myDoctor].prescription;
    print("P_");
    print(itoa(ID, str));
    print(" : Consultation finished and  Got prescription ");
    print(itoa(myPrescription, str));
    print("\n");
    /* Signal the doctor that I have taken the prescription and left */
    Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    Release(doctors[myDoctor].transLock);

    print("P_");
    print(itoa(ID, str));
    print(": Done with Doctor, going to cashier.\n");

    /*////////////////////////////////////////// */
    /*///////  Interaction with Cashier //////// */
    /*////////////////////////////////////////// */

    HLock_Acquire(cashierLineLock);

    print("P_");
    print(itoa(ID, str));
    print(": Acquiring cashierLineLock\n");
    /* find the shortest line */
    sLen = cashiers[0].lineLength;
    if (test_state == 4) {
        print("P_");
        print(itoa(ID, str));
        print(":TEST4: Finding shortest Line of cashiers\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(": Finding shortest Line of cashiers\n");
    }

    for (i = 1; i < numCashiers; ++i) {
        if (test_state == 4) {
            /*Print the length of each line */
            print("P_");
            print(itoa(ID, str));
            print(":TEST4: Length of line at R_");
            print(itoa(i, str));
            print(" = ");
            print(itoa(cashiers[i].lineLength, str));
            print("\n");
        }

        if (cashiers[i].lineLength < sLen) {
            myCashier = i;
            sLen = cashiers[i].lineLength;
        }
    }

    if (test_state == 4) {
        print("P_");
        print(itoa(ID, str));
        print(":TEST4: Found shortest cashier line C_");
        print(itoa(myCashier, str));
        print("len: ");
        print(itoa(sLen, str));
        print("\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(": Found shortest cashier line C_");
        print(itoa(myCashier, str));
        print("len: ");
        print(itoa(sLen, str));
        print("\n");
    }

    /*if(sLen > 0) {get in line} else {get in line} */
    /* there are a lot of cases here, but they all result in us getting in line */
    cashiers[myCashier].lineLength++;
    HDataUpdate_Cash(myCashier, cashiers[myCashier].lineLength,
            cashiers[myCashier].patToken, cashiers[myCashier].fee,
            cashiers[myCashier].payment, cashiers[myCashier].sales);

    print("P_");
    print(itoa(ID, str));
    print(": Waiting in line for cashier C_");
    print(itoa(myCashier, str));
    print(" to attend to me, Line length: ");
    print(itoa(cashiers[myCashier].lineLength, str));
    print("\n");

    HCV_Wait(cashiers[myCashier].lineCV, cashierLineLock);

    print("P_");
    print(itoa(ID, str));
    print(": Going to meet cashier C_");
    print(itoa(myCashier, str));
    print("\n");

    cashiers[myCashier].lineLength--;
    HDataUpdate_Cash(myCashier, cashiers[myCashier].lineLength,
            cashiers[myCashier].patToken, cashiers[myCashier].fee,
            cashiers[myCashier].payment, cashiers[myCashier].sales);

    /* APPROACH THE DESK */
    HLock_Release(cashierLineLock);
    HLock_Acquire(cashiers[myCashier].transLock);

    /* provide token to cashier */
    cashiers[myCashier].patToken = myToken;
    HDataUpdate_Cash(myCashier, cashiers[myCashier].lineLength,
            cashiers[myCashier].patToken, cashiers[myCashier].fee,
            cashiers[myCashier].payment, cashiers[myCashier].sales);

    /* wait for cashier to come back with the fee */
    HCV_Signal(cashiers[myCashier].transCV, cashiers[myCashier].transLock);
    HCV_Wait(cashiers[myCashier].transCV, cashiers[myCashier].transLock);

    /* provide the money */
    cashiers[myCashier].payment = cashiers[myCashier].fee;
    HDataUpdate_Cash(myCashier, cashiers[myCashier].lineLength,
            cashiers[myCashier].patToken, cashiers[myCashier].fee,
            cashiers[myCashier].payment, cashiers[myCashier].sales);

    print("P_");
    print(itoa(ID, str));
    print(": Paying money.\n");

    /* done */
    HCV_Signal(cashiers[myCashier].transCV, cashiers[myCashier].transLock);
    HLock_Release(cashiers[myCashier].transLock);

    print("P_");
    print(itoa(ID, str));
    print(": Done with cashier\n");
    /*//////////////////////////////////////////////// */
    /*//////  Interaction with Pharmacy Clerk //////// */
    /*//////////////////////////////////////////////// */

    print("P_");
    print(itoa(ID, str));
    print(":Attempt to acquire ClerkLinesLock...\n");
    HLock_Acquire(ClerkLinesLock);
    print("success\n");

    shortestclerkline = 0;
    length = clerks[0].patientsInLine;

    if (test_state == 4) {
        print("P_");
        print(itoa(ID, str));
        print(":TEST4: Finding shortest Line of clerks\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(": Finding shortest Line of clerks\n");
    }

    /*Find shortest Line */
    for (i = 0; i < numClerks; i++) {
        if (test_state == 4) {
            /*Print the length of each line */
            print("P_");
            print(itoa(ID, str));
            print(": TEST4: Length of line at CL_");
            print(itoa(i, str));
            print(" = ");
            print(itoa(clerks[i].patientsInLine, str));
            print("\n");
        }
        if (clerks[i].patientsInLine < length) {
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }

    if (test_state != 4) {
        print("P_");
        print(itoa(ID, str));
        print(": Found shortest pharmacy clerk line CL_");
        print(itoa(shortestclerkline, str));
        print(" len: ");
        print(itoa(length, str));
        print("\n");
    } else {
        print("P_");
        print(itoa(ID, str));
        print(":TEST4: Found shortest pharmacy clerk line CL_");
        print(itoa(shortestclerkline, str));
        print(" len: ");
        print(itoa(length, str));
        print("\n");
    }

    /*wait in line for my turn */
    clerks[shortestclerkline].patientsInLine++;
    HDataUpdate_Clerk(shortestclerkline,
            clerks[shortestclerkline].patientsInLine,
            clerks[shortestclerkline].payment, clerks[shortestclerkline].fee,
            clerks[shortestclerkline].patPrescription,
            clerks[shortestclerkline].sales);
    print("P_");
    print(itoa(ID, str));
    print(": Waiting in line for clerk CL_");
    print(itoa(shortestclerkline, str));
    print(" to attend to me, Line length:: ");
    print(itoa(clerks[shortestclerkline].patientsInLine, str));
    print("\n");

    HCV_Wait(clerks[shortestclerkline].ClerkCV, ClerkLinesLock);

    print("P_");
    print(itoa(ID, str));
    print(
            " Got woken up, got out of line and going to the Pharmacy CLerk to give prescription.\n");

    clerks[shortestclerkline].patientsInLine--;
    HDataUpdate_Clerk(shortestclerkline,
            clerks[shortestclerkline].patientsInLine,
            clerks[shortestclerkline].payment, clerks[shortestclerkline].fee,
            clerks[shortestclerkline].patPrescription,
            clerks[shortestclerkline].sales);
    HLock_Release(ClerkLinesLock);
    HLock_Acquire(clerks[shortestclerkline].ClerkTransLock);
    /*signal ParmacyClerk that i am ready to give Prescription */

    print("P_");
    print(itoa(ID, str));
    print(": Acquired ClerkTransLock\n");

    /*Entered the line no need to hold all lines others may now continue */
    /*wait for the PharmacyClerk to Get the prescription from me.. so I wait */
    clerks[shortestclerkline].patPrescription = myPrescription;
    HDataUpdate_Clerk(shortestclerkline,
            clerks[shortestclerkline].patientsInLine,
            clerks[shortestclerkline].payment, clerks[shortestclerkline].fee,
            clerks[shortestclerkline].patPrescription,
            clerks[shortestclerkline].sales);
    print("P_");
    print(itoa(ID, str));
    print(": Gave prescriptiong, waiting for medicines.\n");

    /* wait for clerk to give cost */
    HCV_Signal(clerks[shortestclerkline].ClerkTransCV,
            clerks[shortestclerkline].ClerkTransLock);
    HCV_Wait(clerks[shortestclerkline].ClerkTransCV,
            clerks[shortestclerkline].ClerkTransLock);

    /* provide the money */

    print("P_");
    print(itoa(ID, str));
    print(": Got Medicines, making payment.\n");

    clerks[shortestclerkline].payment = clerks[shortestclerkline].fee;
    HDataUpdate_Clerk(shortestclerkline,
            clerks[shortestclerkline].patientsInLine,
            clerks[shortestclerkline].payment, clerks[shortestclerkline].fee,
            clerks[shortestclerkline].patPrescription,
            clerks[shortestclerkline].sales);
    /* done */
    HCV_Signal(clerks[shortestclerkline].ClerkTransCV,
            clerks[shortestclerkline].ClerkTransLock);

    print("P_");
    print(itoa(ID, str));
    print(": Done with Clerk\n");

    HLock_Release(clerks[shortestclerkline].ClerkTransLock);

    /*7. get out - die die die( ;) muhahahhaha) */
    HLock_Acquire(hospitalLock);

    print("P_");
    print(itoa(ID, str));
    print(" Getting out of the hospital\n");
    peopleInHospital--;
    HGlobalDataUpdate(PEOPLEINHOSPITAL, peopleInHospital);
    HLock_Release(hospitalLock);
}

int main(int argc, char** argv) {
    int i;
    char inp[20];
    char str[20];
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

    numPatients = numberOfEntities[1];
    HLock_Acquire(hospitalLock);
    peopleInHospital = numPatients;
    HLock_Release(hospitalLock);

    print("Creating ");
    print(itoa(numPatients, str));
    print(" Patients\n");

    for (i = 0; i < numPatients; i++)
        Fork(createPatient);

    for (i = 0; i < 100; i++)
        Yield();
}
