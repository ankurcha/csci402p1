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
#include <iostream>
using namespace std;

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

// list mapping patient tokens to consultFees
Lock* feeListLock = new Lock("feeListLock");
linkedlist *feeList = new linkedlist();

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
};

struct Doctor {
    // line lock and CV and protected variables
    Lock *LineLock;
    Condition *LineCV;
    int peopleInLine;
    
    //transaction lock and CV and variables protected
    Lock* transLock;
    Condition* transCV;
    int prescription;
    int patientToken;
    
    Doctor() {
        prescription = -1;
        patientToken = -1;
        
        peopleInLine = 0;
        LineLock = new Lock("LineLock");
        LineCV = new Condition("LineCV");
        
        transLock = new Lock("Doctor.transLock");
        transCV = new Condition("Doctor.transCV");
    }

    ~Doctor() {
        delete transLock;
        delete transCV;
    }
};

// globals to track the queue of doorboys waiting to service doctors
Lock* doorboyLineLock = new Lock("doorboyLineLock");
Condition* doorboyLineCV = new Condition("doorboyLineCV");
int doorboyLineLength = 0;
int wakingDoctorID = 0;

struct DoorBoy {
    Condition *doorboyBreakCV;
    
    DoorBoy(){
        doorboyBreakCV = new Condition("doorboyBreakCV");
    }
};

int MAX_DOCTORS;
int RECP_MAX;
int MAX_PATIENTS;
int MAX_CLERKS;
int MAX_CASHIER;

//TODO: these can't be static -- all pray the dynamic heap gods!!
const int numRecs = 5;
const int numDoctors = 10;
const int numCashiers = 5;
const int numClerks = 5;
//Receptionists receptionists[numRecs];
Receptionists* receptionists = new Receptionists[numRecs];
DoorBoy doorboys[numDoctors];
Doctor doctors[numDoctors];
Cashier cashiers[numCashiers];
PharmacyClerks clerks[numClerks];

int TokenCounter;

Lock *AllLinesLock = new Lock("AllLineLock");
Lock *TokenCounterLock = new Lock("TokenCounterLock");

#include "patient.cc"

void doorboy(int ID){
    int myDoctor = 0;

    while (true) {
        printf("DB_%d: Alive\n",ID);

        //Get into the doorboyLine till some doctor asks for me
        doorboyLineLock->Acquire();

        doorboyLineLength++;
        cout << "DB_"<<ID<<": Waiting for some doctor to wake me up.\n" ;
        doorboyLineCV->Wait(doorboyLineLock);
        doorboyLineLength--;
        
        //Some doctor woke me up, lets check who
        myDoctor =  wakingDoctorID;
        doorboyLineLock->Release();

        // Inform the doctor that I have arrived, and wait for him to take 
        //  a break, if he so chooses
        doctors[myDoctor].transLock->Acquire();
        doctors[myDoctor].transCV->Signal(doctors[myDoctor].transLock);
        doctors[myDoctor].transCV->Wait(doctors[myDoctor].transLock);
        
        ///// PATIENT LINE /////
        //Acquire the lock to get the state of the line and take decision
        doctors[myDoctor].LineLock->Acquire();
        printf("DB_%d: Checking for Patients\n",ID);
        
        //while there is noone in line
        while(doctors[myDoctor].peopleInLine <= 0) { 
            //I will be woken up by the manager only!!
            printf("DB_%d: Yawn!!...ZZZZzzzzz....\n",ID);
            doorboys[ID].doorboyBreakCV->Wait(doctors[myDoctor].LineLock);
            // I got woken up, time to go back to work - by now there are 
            //  people dying on the floor!
        }
        
        printf("DB_%d: Found %d patients waiting in line for D_%d\n",
                ID, doctors[myDoctor].peopleInLine, myDoctor);
    
        //Now wake the patient up to go to the doctor
        printf("DB_%d: Tell patient to go to doctor D_%d\n", ID, myDoctor);
        doctors[myDoctor].LineCV->Signal(doctors[myDoctor].LineLock);

        //My job with the patients and the doctor is done
        //I can go back on the doorboyLine
        doctors[myDoctor].LineLock->Release();
        
    }//End of while

    printf("DB_%d: Dying...AAAaaaahhhhhhhhh!!\n",ID);
}

void doctor(int ID){
    while(true) {
        // acquire a doorboy
        doorboyLineLock->Acquire();

        // assure that there is a doorboy in line
        while(doorboyLineLength <= 0) {
            printf("Doctor could not find a doorboy!\n");
            doorboyLineLock->Release();
            currentThread->Yield();
            doorboyLineLock->Acquire();
        }
        
        // pull the next doorboy off the line
        wakingDoctorID = ID;
        doorboyLineCV->Signal(doorboyLineLock);

        // acquire the transaction lock and wait for the doorboy to arrive
        doctors[ID].transLock->Acquire();
        doorboyLineLock->Release();

        //////  DOORBOY INTERACTION  //////
        doctors[ID].transCV->Wait(doctors[ID].transLock);

        // go on break if so inclined
        if(Random() % 100 > 49) { // go on break
            // 5-15 yields
            int numYields = 5 + (Random() % 11);
            for(int i=0; i < numYields; ++i) {
                currentThread->Yield();
            }
        }

        // inform the doorboy that I am ready for a patient
        doctors[ID].transCV->Signal(doctors[ID].transLock);

        //////  PATIENT INTERACTION  //////
        // and wait for that patient to arrive
        doctors[ID].transCV->Wait(doctors[ID].transLock);

        // consult: 10-20 yields
        int numYields = 10 + (Random() % 11);
        for(int i=0; i < numYields; ++i) {
            currentThread->Yield();  // I see ... mm hmm ... does it hurt here? ...
        }

        // give prescription to patient
        doctors[ID].prescription = Random() % 100;

        // put consultation fees into the data structure for the cashier ($50-$250)
        int consultFee = 50 + (Random() % 201);
        feeListLock->Acquire();
        feeList->append(doctors[ID].patientToken, consultFee);
        feeListLock->Release();

        // pass the prescription to the patient and wait for them to leave
        doctors[ID].transCV->Signal(doctors[ID].transLock);
        doctors[ID].transCV->Wait(doctors[ID].transLock);

        // done, the patient has left
        doctors[ID].transLock->Release();

    } //end while
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
        int patientsWaiting=0;
        for (int j=0; j<RECP_MAX; j++) {
            patientsWaiting += receptionists[j].peopleInLine;
        }
        
        for (int i=0; i<RECP_MAX; i++) {//Check for waiting patients
            if (receptionists[i].peopleInLine > 0 && receptionists[i].state == SLEEPING) {
                printf("H_%d : found R_%d sleeping and %d waiting\n",ID,i,receptionists[i].peopleInLine);
                    //Wake up this receptionist up
                    //receptionists[ID].ReceptionistBreakLock->Acquire();
                receptionists[ID].LineLock->Acquire();
                receptionists[ID].ReceptionistBreakCV->Signal(receptionists[ID].LineLock);
                receptionists[ID].LineLock->Release();
                
            }
        }
            //2. Query Cashiers
            //3. Query pharmacy
    }
}



//void INIT(){
//    
//    int i = 0;
//    char *temp;
//    Thread *t;
//    
//    t = new Thread("patient_0");
//    t->Fork((VoidFunctionPtr) patients, 0);
//    
//    t = new Thread("patient_1");
//    t->Fork((VoidFunctionPtr) patients, 1);
//    
//    t = new Thread("patient_2");
//    t->Fork((VoidFunctionPtr) patients, 2);
//    
//    t = new Thread("receptionist_0");
//    t->Fork((VoidFunctionPtr) receptionist, 0);
//    
//    t = new Thread("doorboy_0");
//    t->Fork((VoidFunctionPtr) doorboy, 0);
//    
//}

void HospINIT() {
    
    int totalHospMan = 1;
    int MAXDOCTORS=10;
    int MINDOCTORS=4;
    int MAX_DOORB;
    int MAXRCP=3;
    int MINRCP=5;
    int MAXPAT=100;
    int MINPAT=20;
    
    
    int i = 0;
    char *temp = new char[100];
    Thread *t;
    
      
        //2. Receptionists
    RECP_MAX= Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    for(i=0;i<RECP_MAX;i++)
    {
        sprintf(temp,"Receptionist_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) receptionist, i);
    }
    
        //3. Cashiers
    MAX_CASHIER = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    for(i=0;i<MAX_CASHIER;i++)
    {
        sprintf(temp,"Cashier_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) cashier, i);
    }
    
        //4. DoorBoys
    MAX_DOORB = MAX_DOCTORS;
    for(i=0;i<MAX_DOORB;i++)
    {
        sprintf(temp,"DoorBoy_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) doorboy, i);
    }
    
        //5. Pharmacys
    MAX_CLERKS= Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    for(i=0;i<MAX_CLERKS;i++)
    {
        sprintf(temp,"PharmacyClerk_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) Clerk, i);
    }
        //1. Doctors
    MAX_DOCTORS = Random() % (MAXDOCTORS - MINDOCTORS + 1) + MINDOCTORS;
    for(i=0;i<MAX_DOCTORS;i++)
    {
        sprintf(temp,"Doctor_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) doctor, i);
    }
    
    
        //6. HospitalManager
    t = new Thread("HospitalManager_0");
    t->Fork((VoidFunctionPtr) hospitalManager, 0);
    
        //7. Patients
    MAX_PATIENTS = Random() % (MAXPAT - MINPAT +1) + MINPAT ;
    for(i=0;i<MAX_PATIENTS;i++)
    {
        sprintf(temp,"Patients_%d",i);
        t=new Thread(temp);
        t->Fork((VoidFunctionPtr) patients, i);
    }
}
