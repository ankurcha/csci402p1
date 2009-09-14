
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
}h;

struct Patient{
    int tokenNumber ;
    int doctor_id ;
    int prescription;
    int fees;
};

int tokencount = 0;
Lock *R2RLock = new Lock("R2RLock");


SynchList *waitqueue = new SynchList;
int tokenCounter= 0;
Lock *tokenCounterLock = new Lock("TokenCounterLock");

Lock *waitPatients = new Lock("Patients Waiting for their token Number");
Condition *waitPatientsCV = new Condition("WaitingPatientsCV");

int numWaiting = 0;
Lock *numWaitingLock = new Lock("Num Waiting Lock");

Lock *receptionistSleeping = new Lock("ReceptionistSleepingLock");
Condition *receptionistSleepingCV = new Condition("receptionistSleepingCV");

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

Lock *R2PLock = new Lock("R2PLock");
Condition *R2PLockCV = new Condition("R2PLockCV");
Condition *R2RLockCV = new Condition("R2RLockCV");

int availableTokens;
int neededTokens = 0;
void patient(int id){

  R2PLock->Acquire();
  neededTokens++;
  if(availableTokens <= 0){
    printf("\n%s going to wait...\n",currentThread->getName());
    R2PLockCV->Wait(R2PLock);
    printf("\n%s back to Life!!",currentThread->getName());
  }
  printf("\n%s: I'm awake",currentThread->getName());
  Patient me;
  me.tokenNumber = tokencount;
  //neededTokens--;
  availableTokens--;
  printf("\n%s: Acquired Token=%d\n",currentThread->getName(),me.tokenNumber);
  R2PLock->Release();
  printf("\n%s Exiting\n",currentThread->getName());
}


void receptionist(int id){

  while(neededTokens>0){
    R2RLock->Acquire();
    R2PLock->Acquire();
    printf("\n%s: Generating token = %d\n",currentThread->getName(),++tokencount);
    availableTokens++;
    neededTokens--;
    //Signal a waiting patient on a R2PLock
    printf("\n%s: Sending Signal to Patients\n",currentThread->getName());
    R2PLockCV->Signal(R2PLock);
    }
  //R2RLock->Release();
    //R2RLock->Acquire();
  R2RLockCV->Wait(R2RLock);
  R2RLock->Release();
}

void HospINIT(){
    Thread *t;
    char *name;
    int i;

    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("patient_0");
    t->Fork((VoidFunctionPtr) patient, 0);

    t = new Thread("patient_1");
    t->Fork((VoidFunctionPtr) patient, 1);
    
    t = new Thread("patient_2");
    t->Fork((VoidFunctionPtr) patient,2);

    t = new Thread("receptionist_0");
    t->Fork((VoidFunctionPtr) receptionist, 0);


}
