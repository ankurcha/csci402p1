
#define MINPATIENTS 20
#define MAXPATIENTS 100


#include "list.h"
#include "synchlist.h"
#define MINDOCTORS 4
#define MAXDOCTORS 10
#define MINRCP 3
#define MAXRCP 5

struct Hospital{
    int totalDocs;
    int totalRecp;
    int totalCash;
    int totalDoor;
    int totalPharm;
    int totalHospMan;
    int totalPatients;
//    //SynchList *TokenQueue;
//    Lock *R2R_tokencounter;//Lock to access tokencounter
//    int tokenCounter = 0;
//    Lock *P2P_token;//Lock to get the newly assigned token number
//    Lock *waitqueue;
//    Patient *patients;
}h;

struct Patient{
    int tokenNumber = -1;
    int doctor_id = -1;
    int prescription = -1;
    int fees = -1;
};

SynchList *waitqueue = new SynchList;
int tokenCounter= 0;
Lock *tokenCounterLock = new Lock("TokenCounterLock");

Lock *waitPatients = new Lock("Patients Waiting for their token Number");
Condition *waitPatientsCV = new Condition("WaitingPatientsCV");

int numWaiting = 0;
Lock *numWaitingLock = new Lock("Num Waiting Lock");

//int waitingPatients = 0; //Shared variable use with rwPlock
//Lock *rwPlock = new Lock("Reception Waiting Lock");
//Condition *reception = new Condition("waitingPatients CV");
//
//int tokenPass = -1;
//Lock *tokenPassLock = new Lock("Token Get Lock");
//Condition *tokenPassCV = new Condition("TokenPass CV");

void initHospital(){
    //h.TokenQueue = new SynchList;

    h.totalDocs = Random() % (MAXDOCTORS - MINDOCTORS + 1) + MINDOCTORS;
    //2. Receptionists
    h.totalRecp = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    //3. Cashiers
    h.totalCash = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    //4. DoorBoys
    h.totalDoor = h.totalDocs;
    //5. Pharmacys
    h.totalPharm = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    //6. HospitalManager
    h.totalHospMan = 1;
    //7. Patients
    h.totalPatients = Random() % (MAXPATIENTS - MINPATIENTS +1) + MINPATIENTS ;
    //h.patients = new Patient[h.totalPatients];
    
}

void patient(){
//    //TODO: Code for the patient
//    //1. Come to hospital
//    printf("Patient:%s :Arrived at the Hospital",patient_id);
//    Patient *me = new Patient();
//    //2. Meet receptionist
//    //  2.1 Stand in queue to be allocated token number
//    rwPlock->Acquire();
//        waitingPatients++; //increment the count of patients waiting
//    rwPlock->Release();
//    reception->Wait(h.waitqueue); // Wait for token to be set into tokenPass
//
//    //Token has been set and I am waiting for it.
//
//
//
//    //  2.2 Goto sleep till someone wakes you up and gives you a token number
//    printf("Patient:%s : Aquired Token %d",patient_id,me->tokenNumber);
//    //3. Generate a number to select doctor
//    int selected_doctor = Random() % h.totalDocs;
//    //  3.1 Wait in line to meet doctor
    printf("Patient:%s :Arrived at the Hospital",currentThread->getName());
    Patient *me = new Patient();
    me->tokenNumber = -1;
    numWaitingLock->Acquire();
        numWaiting++;
    numWaitingLock->Release();
    //Wait for token number from receptionist
    waitPatients->Acquire();
    waitPatientsCV->Wait(waitPatients);
        //Now the token Number should be available
    me->tokenNumber = tokenCounter;
    printf("Patient:%s : Aquired Token %d",currentThread->getName(),me->tokenNumber);

}


void receptionist(){
//    int token = Random()%h.totalPatients;
//    while(waitingPatients > 0){
//        token = Random()%h.totalPatients;
//        //wake a patient up and give it a token number
//
//        //patient->tokenNumber = token;
//        rwPlock->Acquire();
//        waitingPatients--;
//        rwPlock->Release();
//
//    }

    numWaitingLock->Acquire();
    if(numWaiting>0){
        //There are ppl waiting for a lock
        //Acquire lock for the token counter
        tokenCounterLock->Acquire();
            tokenCounter++;
        tokenCounterLock->Release();

        //Wake up patient
        waitPatientsCV->Signal(waitPatients);
        numWaiting--;
    }
    numWaitingLock->Release();
}