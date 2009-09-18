/*
 *  init.cpp
 *
 *  Hospital management simulation threads
 *  
 *
 *  Created by Ankur Chauhan on 9/15/09.
 *  USC CSCI 402 Operating Systems
 *  Group 11
 *    Ankur Chauhan, ankurcha
 *    Max Pflueger, pflueger
 *    Aneesha Mathew, aneesham
 *
 */

#include "system.h"

#define BUSY 0
#define FREE 1
#define SLEEPING 2

struct node {
    int key, value;
    node* next;
};


struct linkedlist { 
//Used for storing the <token,fees> pairs
    node* head;
    int length;
    
    void append(int key, int val){
        if (head == NULL) {
            head = new node;
            head->key = key;
            head->value = val;
            head->next = NULL;
            this->length++;
        }else {
            node *p = new node;
            p->key = key;
            p->value = val;
            p->next = head;
            head = p;
            this->length++;
        }
    }
    
    int getValue(int key){
        node *p = head;
        if(head!=NULL){
            while (p!=NULL) {
                if (p->key == key) {
                    return p->value;
                }else {
                    p = p->next;
                }
            }//End of While
        }else{
                //empty list
            return -1;
        }
    }
};


//shared data struct related to a Receptionist

struct Receptionists{
    int state;

    int peopleInLine;
    
    Lock *LineLock;
    Condition *receptionCV;
    Condition *receptionistWaitCV; //Wait using LineLock
    Condition *ReceptionistBreakCV; //Break using LineLock

    int currentToken;
    
    
    
    Receptionists(){
        peopleInLine = 0;
        state = FREE;

        LineLock = new Lock("LineLock");
        receptionCV = new Condition("receptionCV");
        receptionistWaitCV = new Condition("receptionistWaitCV");
        ReceptionistBreakCV = new Condition("ReceptionistBreakCV");
            //        ReceptionistBreakLock = new Lock("ReceptionistBreakLock");
        currentToken = 0;
    }
};

// global for all cashiers
Lock* cashierLineLock = new Lock("cashierLineLock");
Lock* feesPaidLock = new Lock("feesPaidLock");
int feesPaid = 0;

// shared data struct related to a Cashier
struct Cashier {
    // line CV and length
    int lineLength;
    Condition* lineCV;

    // transaction lock, CV, and variables protected by the former
    Lock* transLock;
    Condition* transCV;
    int patToken;
    int fee;
    int payment;

    // cashier's CV for going on break
    Condition* breakCV;

    Cashier() {
        lineLength = 0;
        patToken = 0;
        fee = 0;
        payment = 0;

        lineCV = new Condition("Cashier.lineCV");
        transLock = new Lock("Cashier.transLock");
        transCV = new Condition("Cashier.transCV");
        breakCV = new Condition("Cashier.breakCV");
    }

    ~Cashier() {
        delete lineCV;
        delete transLock;
        delete transCV;
        delete breakCV;
    }
};

Lock *ClerkLinesLock= new Lock("ClerkLineLock");
Lock *PaymentLock= new Lock("PaymentLock");
int totalsales=0;

struct PharmacyClerks{
	int patientsInLine;
	int state;
	int payment;
	int fee;
	int patPrescription;
	Condition *ClerkCV;
	
	Condition *ClerkBreakCV;
	Lock* ClerkTransLock;
    Condition* ClerkTransCV;
  
  
  PharmacyClerks(){
  	patientsInLine= 0;
  	state=FREE;
  	payment=0;
  	fee=20;
  	patPrescription=0;
  ClerkCV= new Condition("ClerkCV");
  ClerkBreakCV=new Condition("ClerkBreakCV");
 ClerkTransLock=new Lock("ClerkTransLock");
  ClerkTransCV=new Condition("ClerkTransCV");
}  
}clerks[3];

struct Doctor {
    int state;
    int currentPrescription;
    int currentFees;
    int currentPatientToken;
    Lock *DoctorStateChangeLock;
    
    Lock *patientRespondLock;
    Condition *patientRespondCV;
    
    Lock *patientPrescriptionLock;
    Condition *patientPrescriptionCV;
    
    Lock *doctorBreakLock;
    Condition *doctorBreakCV;
    
    Doctor(){
        state = FREE;
        currentPatientToken = -1;
        currentPrescription = -1;
        currentFees = -1;
        DoctorStateChangeLock = new Lock("DoctorStateChangeLock");
        patientRespondLock = new Lock("patientRespondLock");
        patientRespondCV = new Condition("patientRespondCV");
        patientPrescriptionLock = new Lock("patientPrescriptionLock");
        patientPrescriptionCV = new Condition("patientPrescriptionCV");
        doctorBreakLock = new Lock("doctorBreakLock");
        doctorBreakCV = new Condition("doctorBreakLock"); 
    }
};

struct DoorBoy {
    int state;
    int peopleInLine;
    
    Lock *LineLock;
    Condition *LineCV;
    
    Lock *doorboyWaitLock;
    Condition *doorboyWaitCV;
    
    Lock *doorboyBreakLock;
    Condition *doorboyBreakCV;
    
    DoorBoy(){
        state = FREE;
        peopleInLine = 0;
        LineLock = new Lock("LineLock");
        LineCV = new Condition("LineCV");
        doorboyWaitLock = new Lock("doorboyWaitLock");
        doorboyWaitCV = new Condition("doorboyWaitCV");
        doorboyBreakLock = new Lock("doorboyBreakLock");
        doorboyBreakCV = new Condition("doorboyBreakCV");
    }
};

int MAX_DOCTORS;
int RECP_MAX;
int MAX_PATIENTS;
int MAX_CLERKS;

//TODO: these can't be static -- all pray the dynamic heap gods!!
Receptionists receptionists[3];
DoorBoy doorboys[3];
Doctor doctors[3];
const int numCashiers = 3;
Cashier cashiers[numCashiers];

int TokenCounter;

Lock *AllLinesLock = new Lock("AllLineLock");
Lock *TokenCounterLock = new Lock("TokenCounterLock");

#include "patient.cc"

void doorboy(int ID){
    while (true) {
        printf("DB_%d: Alive\n",ID);
        doorboys[ID].state = FREE;
            //Acquire the lock to get the state of the line and take decision
        doorboys[ID].LineLock->Acquire();
        printf("DB_%d: Checking for Patients\n",ID);
        if (doorboys[ID].peopleInLine > 0) {
            doctors[ID].DoctorStateChangeLock->Acquire();
                //In case the doctor is sleeping just wait till he is back and
                //only then proceed.
            if (doctors[ID].state == SLEEPING) {
                printf("DB_%d: Checking for the doctor\n",ID);
                doctors[ID].DoctorStateChangeLock->Release();
                doorboys[ID].LineLock->Release();
                continue;
            }
            printf("DB_%d: Found %d people on my line\n",ID,doorboys[ID].peopleInLine);
                //Service them
                //The doorboy discovers that the patient is waiting and then
                //goes onto wait for the doctor to give the word to send the
                //next person in.
                //Let others enter the queue, so release the lock
            doorboys[ID].LineLock->Release();
                //Acquire the wait-for-the-doctor lock
            doorboys[ID].doorboyWaitLock->Acquire();
                //Wait for the doctor to call for the next person
            doorboys[ID].doorboyWaitCV->Wait(doorboys[ID].doorboyWaitLock);
                //OK, now I got the signal from the doctor to send the next
                //person in, I have to tell this to the patient so that he can
                //proceed to the doctor
            doctors[ID].patientRespondLock->Acquire();
            doorboys[ID].doorboyWaitLock->Release(); // I don't need to wait for
                                                     //the doctor now!!
            //Tell the patient to get up and go see the doctor
            doctors[ID].patientRespondCV->Signal(doctors[ID].patientRespondLock);
                //The patient will now go in, I can release the lock
            doctors[ID].patientRespondLock->Release();
                //All my job is done, I'll go after something more meaningful in
                //life now!!
            continue;
            
        }else {
                //No one to service for my doctor, check the state of the doctor
            printf("DB_%d: Found no one in the line, checking to see if I can go sleep\n",ID);
            doctors[ID].DoctorStateChangeLock->Acquire();
            if (doctors[ID].state == SLEEPING) {
                doorboys[ID].LineLock->Release();//Let others get into the queue
                    //The doorboy cannot go on a break when the doctor is not there
                doctors[ID].DoctorStateChangeLock->Release();
                continue;
            }else {
                    //The doctor is in either in BUSY or FREE, then I can go on a break
                doorboys[ID].doorboyBreakLock->Acquire();
                doorboys[ID].state = SLEEPING;
                doorboys[ID].LineLock->Release(); // Let others enter the queue
                doctors[ID].DoctorStateChangeLock->Release();
                printf("DB_%d: Yawn!!...ZZZZzzzzz....\n",ID);
                doorboys[ID].doorboyBreakCV->Wait(doorboys[ID].doorboyBreakLock);
                    //I will be woken up by the manager only!!
                    //OK I got woken up, Time to release locks, change state and
                    //go back to work - by now there are people dying on the floor!
                doorboys[ID].doorboyBreakLock->Release();
                continue;
            }

            
        }

        
    }
    printf("DB_%d: Dying...AAAaaaahhhhhhhhh!!\n",ID);
}

void doctor(int ID){
        //TODO: Someone needs to do this....I wonder who??? The Fairy thread maybe!!
}

void receptionist(int ID){
    while (true) {
        AllLinesLock->Acquire();
        receptionists[ID].state = FREE;
        if (receptionists[ID].peopleInLine > 0) {
            //Some one in my line - service them, first generate new token
            receptionists[ID].LineLock->Acquire();
            receptionists[ID].state = BUSY;
            //Genetate token for the patient
            //Acquire token lock
            TokenCounterLock->Acquire();
            //Increment token Counter
                //Take token for patient
            printf("R_%d: Generating Token...\n",ID);
            receptionists[ID].currentToken = ++TokenCounter;
                //New Token Available with the receptionist
            TokenCounterLock->Release();
                //Wake one waiting patient up
            receptionists[ID].receptionCV->Signal(receptionists[ID].LineLock);            
            //Sleep till you get Acknowledgement
            receptionists[ID].receptionistWaitCV->Wait(receptionists[ID].LineLock);
                        AllLinesLock->Release();
            //Patient successfully got the token, go back to work: Loop again
            printf("R_%d: Continue to next Patient\n",ID);
            receptionists[ID].LineLock->Release();
        }else {
            //My Line is empty
            DEBUG('t',"No Patients, going on break...");
            printf("R_%d Going to sleep\n",ID);
            receptionists[ID].state = SLEEPING;
            receptionists[ID].LineLock->Acquire();
            receptionists[ID].ReceptionistBreakCV->Wait(receptionists[ID].LineLock);
            receptionists[ID].LineLock->Release();
            //HospitalManager kicked my ass for sleeping on the job!!
            //Loop back!!
            continue;
        }
    }
}

void cashier(int ID) {
    while(true) {
        cashierLineLock->Acquire();
        
        if(cashiers[ID].lineLength > 0) { // someone in line
            //signal person on top
            cashiers[ID].lineCV->Signal(cashierLineLock);
        } else { // noone in line
            // go on break
            cashiers[ID].breakCV->Wait(cashierLineLock);
            cashierLineLock->Release();
            continue;
        }
        
        // I have a patient
        // acquire transLock and use it to govern transactions
        //  with the patient
        cashiers[ID].transLock->Acquire();
        cashierLineLock->Release();

        // waiting for patient to deposit its token in patToken
        cashiers[ID].transCV->Wait(cashiers[ID].transLock);

        //TODO lookup value for cashiers[ID].patToken in the token table
        cashiers[ID].fee = 42;
        // tell patient the fee
        
        cashiers[ID].transCV->Signal(cashiers[ID].transLock);
        // wait for payment
        cashiers[ID].transCV->Wait(cashiers[ID].transLock);

        // add this payment to our total collected
        feesPaidLock->Acquire();
        feesPaid += cashiers[ID].payment;
        feesPaidLock->Release();
        if(cashiers[ID].payment < cashiers[ID].fee) {
            printf("ERROR: call security, that patient didin't pay!");
        }        

        cashiers[ID].transLock->Release();
    }
}


void Clerk(int ID){
	while(true){
		ClerkLinesLock->Acquire();
        
        if(clerks[ID].patientsInLine > 0) { // someone in line
            //signal the first person
            clerks[ID].ClerkCV->Signal(ClerkLinesLock);
        } else { // noone in line
            // go on break
            clerks[ID].ClerkBreakCV->Wait(ClerkLinesLock);
            ClerkLinesLock->Release();
            continue;
        }
        
        // I have a patient
        // acquire the transaction Lock for further transactions
        //  with the patient
        clerks[ID].ClerkTransLock->Acquire();
        ClerkLinesLock->Release();

        // waiting for patient to give prescription
        clerks[ID].ClerkTransCV->Wait(clerks[ID].ClerkTransLock);

         // patient gives prescription:
         printf("C_%d: gave Medicines!\n",ID);
        //TODO: lookup the total cost with priscription
                        
        clerks[ID].ClerkTransCV->Signal(clerks[ID].ClerkTransLock);
        // wait for payment
        clerks[ID].ClerkTransCV->Wait(clerks[ID].ClerkTransLock);
        //Collect payment

        // add this payment to our total collected
        PaymentLock->Acquire();
        totalsales += clerks[ID].payment;
        PaymentLock->Release();
             

        clerks[ID].ClerkTransLock->Release();
	}
	}

void hospitalManager(int ID){
    printf("H_%d : Alive",ID);
    int sleeptime = Random() % 30;
    while (true) {
        sleeptime = Random() % 30;
            //Sleep for some random amount of time
        printf("H_%d : Sleeping for %d cycles\n",ID,sleeptime);
        while (sleeptime > 0) {
            currentThread->Yield();
        }
            //I am on rounds now, Time to kick some ass
        printf("H_%d : Going on rounds\n",ID);
            //1. Check on the Receptionists
        printf("H_%d : Checking receptionists\n",ID);
        for (int i=0; i<RECP_MAX; i++) {//Check for waiting patients
            if (receptionists[i].peopleInLine > 0 && receptionists[i].state == SLEEPING) {
                printf("H_%d : found R_%d sleeping and %d waiting\n",ID,i,receptionists[i].peopleInLine);
                    //Wake up this receptionist up
                    //receptionists[ID].ReceptionistBreakLock->Acquire();
                receptionists[ID].LineLock->Acquire();
                receptionists[ID].ReceptionistBreakCV->Broadcast(receptionists[ID].LineLock);
                receptionists[ID].LineLock->Release();
                
            }
        }
            //2. Query Cashiers
            //3. Query pharmacy
    }
}



void INIT(){
    
    int i = 0;
    char *temp;
    Thread *t;
    
    t = new Thread("patient_0");
    t->Fork((VoidFunctionPtr) patients, 0);
    
    t = new Thread("patient_1");
    t->Fork((VoidFunctionPtr) patients, 1);
    
    t = new Thread("patient_2");
    t->Fork((VoidFunctionPtr) patients, 2);
    
    t = new Thread("receptionist_0");
    t->Fork((VoidFunctionPtr) receptionist, 0);
    
    t = new Thread("doorboy_0");
    t->Fork((VoidFunctionPtr) doorboy, 0);
    
}

void HospINIT() {
    
    MAX_DOCTORS = 1;
    RECP_MAX = 1;
    MAX_PATIENTS = 3;
    MAX_CLERKS =3;
    
    INIT(); // initiate threads
}
