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


struct linkedlist { //Used for storing the <token,fees> pairs
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
    
    Condition *receptionCV;

    Lock *LineLock;
    int peopleInLine;

    Lock *receptionistWaitLock;
    Condition *receptionistWaitCV;
    int currentToken;
    
    Lock *ReceptionistBreakLock;
    Condition *ReceptionistBreakCV;
    
    Receptionists(){
        peopleInLine = 0;
        state = FREE;
        receptionCV = new Condition("receptionCV");
        LineLock = new Lock("LineLock");
        receptionistWaitLock = new Lock("receptionistWaitLock");
        receptionistWaitCV = new Condition("receptionistWaitCV");
        ReceptionistBreakCV = new Condition("ReceptionistBreakCV");
        ReceptionistBreakLock = new Lock("ReceptionistBreakLock");
        currentToken = 0;
    }
};

// shared data struct related to a Cashier
struct Cashier {
    int state;
    
    Lock* lineLock;
    int lineLength;
    Condition* lineCV;

    Lock* transLock;
    Condition* transCV;
    int patToken;

    Condition* breakCV;

    Cashier() {
        lineLength = 0;
        state = FREE;
        lineLock = new Lock("Cashier.lineLock");
        lineCV = new Condition("Cashier.lineCV");
        transLock = new Lock("Cashier.transLock");
        transCV = new Condition("Cashier.transCV");
        breakCV = new Condition("Cashier.breakCV");
    }

    ~Cashier() {
        delete lineLock;
        delete lineCV;
        delete transLock;
        delete transCV;
        delete breakCV;
    }
};

struct Doctor {
    int state;
    int currentPrescription;
    int currentFees;
    int currentPatientToken;
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

//TODO: these can't be static
Receptionists receptionists[3];
DoorBoy doorboys[3];
Doctor doctors[3];
Cashier cashiers[3];

int TokenCounter;

Lock *AllLinesLock = new Lock("AllLineLock");
Lock *TokenCounterLock = new Lock("TokenCounterLock");

void patients(int ID){
    int myToken;
    int myDoctor;
    int myPrescription;
    printf("P_%d:Attempt to acquire AllLinesLock...",ID);
    AllLinesLock->Acquire();
    printf("success\n");
    int shortestline = 0;
    int len = 0;
    //Find shortest Line
    for (int i=0; i<RECP_MAX; i++) {
        if(receptionists[i].peopleInLine < len){
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    printf("P_%d: found shortest line R_%d len: %d\n",ID,shortestline,len);
    if (len >0) {
        //wait in line for my turn
        receptionists[shortestline].peopleInLine++;
        receptionists[shortestline].LineLock->Acquire();
        AllLinesLock->Release();
        receptionists[shortestline].receptionCV->Wait(receptionists[shortestline].LineLock);
        printf("P_%d: AllLinesLock Released, Now Waiting for signal by Receptionist\n",ID);
    }else { //No one else in line
        switch (receptionists[shortestline].state) {
            case FREE:
            case BUSY:
            case SLEEPING:
                //wait in line
                receptionists[shortestline].peopleInLine++;
                receptionists[shortestline].LineLock->Acquire();
                //Entered the line no need to hold all lines others may now continue
                AllLinesLock->Release();
                receptionists[shortestline].receptionCV->Wait(receptionists[shortestline].LineLock);
                printf("P_%d: AllLinesLock Released, Now Waiting for signal by Receptionist\n",ID);
                break;
            default:
                break;
        }
    }
    
    printf("P_%d Got woken up, get out of line and going to counter for token\n",ID);
    receptionists[shortestline].peopleInLine--;
    //signal receptionist that i am ready to take the token
    
    //wait for the receptionist to prepare token for me, till then I wait
    //token is ready just read it -- print it out in our case
    myToken = receptionists[shortestline].currentToken;
    printf("P_%d: My token is %d\n",ID,myToken);
    //Done, signal receptionist that he can proceed 
    receptionists[shortestline].receptionistWaitLock->Acquire();
    receptionists[shortestline].receptionistWaitCV->Signal(receptionists[shortestline].receptionistWaitLock);
    receptionists[shortestline].receptionistWaitLock->Release();
    //Release Line Lock
    receptionists[shortestline].LineLock->Release();
    
    //Calculate which doctor I want to see
    myDoctor = Random() % MAX_DOCTORS;
    printf("P_%d : Going to meet doctor D_%d\n",ID,myDoctor);
    //1. Acquire doc's line lock
    doorboys[myDoctor].LineLock->Acquire();
    //2. Wait on the line -- to be woken up by the bell boy
    printf("P_%d : Waiting for doorboy to tell me to go\n",ID);
    doorboys[myDoctor].LineCV->Wait(doorboys[myDoctor].LineLock);
    
    //doctor told the door boy to wake me up for consultation, he is waiting for me to respond
    //Now I have to provide my token numeber to the doctor as he is ready for me, I must acquire
    //lock for that and then provide all the information befor i proceed
    doctors[myDoctor].patientRespondLock->Acquire();
    //I can release the line lock so that other people may also join in
    doorboys[myDoctor].LineLock->Acquire();
    //The doctor is waiting for me to provide my info, oblige him!!
    printf("P_%d : Consulting Doctor D_%d now...\n",ID,myDoctor);
    doctors[myDoctor].currentPatientToken = myToken;
    doctors[myDoctor].patientRespondCV->Wait(doctors[myDoctor].patientRespondLock);
    //Consultation in process....
    //4. Consultation finished, now I have to get the prescription from the doctor
    //The doctor would be waiting for me to take this
    printf("P_%d : Consultation finished!!\n",ID);
    doctors[myDoctor].patientPrescriptionLock->Acquire();
    myPrescription = doctors[myDoctor].currentPrescription;
    printf("P_%d : Got prescription# %d\n",ID,myPrescription);
    //Signal the doctor that I have taken the prescription
    doctors[myDoctor].patientPrescriptionCV->Signal(doctors[myDoctor].patientPrescriptionLock);
    doctors[myDoctor].patientRespondLock->Release();
    //5. goto cashier similar to 1 - 5
    //---WAITING FOR MAX TO FILL IN-----
    //6. goto pharmacy  1 - 5
    //7. get out - die die die( ;) )
}

void doctor(int ID){
    
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
            TokenCounter++;
            receptionists[ID].currentToken = TokenCounter;
            //New Token Available with the receptionist
            TokenCounterLock->Release();
            //Wake one waiting patient up
            receptionists[ID].receptionistWaitLock->Acquire();
            receptionists[ID].receptionCV->Signal(receptionists[ID].LineLock);
            AllLinesLock->Release();
            receptionists[ID].LineLock->Release();
            //Sleep till you get Acknowledgement
            receptionists[ID].receptionistWaitCV->Wait(receptionists[ID].receptionistWaitLock);
            //Patient successfully got the token, go back to work: Loop again
            receptionists[ID].receptionistWaitLock->Release();
        }else {
            //My Line is empty
            DEBUG('t',"No Patients, going on break...");
            printf("R_%d Going to sleep\n",ID);
            receptionists[ID].state = SLEEPING;
            receptionists[ID].ReceptionistBreakLock->Acquire();
            receptionists[ID].ReceptionistBreakCV->Wait(receptionists[ID].ReceptionistBreakLock);
            //HospitalManager kicked my ass for sleeping on the job!!
            receptionists[ID].ReceptionistBreakLock->Release();
            //Loop back!!
        }
    }
}

void cashier(int ID) {
    while(true) {
        cashiers[ID].lineLock->Acquire();
        
        if(cashiers[ID].lineLength > 0) { // someone in line
            //signal person on top
            cashiers[ID].lineCV->Signal(cashiers[ID].lineLock);
        } else { // noone in line
            // go on break
            cashiers[ID].breakCV->Wait(cashiers[ID].lineLock);
            cashiers[ID].lineLock->Release();
            continue;
        }
        
        // I have a patient
        // acquire transLock and use it to govern transactions
        //  with the patient
        cashiers[ID].transLock->Acquire();
        cashiers[ID].lineLock->Release();

        // waiting for patient to deposit its token in patToken
        cashiers[ID].transCV->Wait(cashiers[ID].transLock);

        //TODO lookup value for cashiers[ID].patToken in the token table
        //TODO tell patient the fee
        
        cashiers[ID].transCV->Signal(cashiers[ID].transLock);
        // wait for payment
        cashiers[ID].transCV->Wait(cashiers[ID].transLock);

        //TODO add this payment to our total collected
        
        cashiers[ID].transLock->Release();
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
                receptionists[ID].ReceptionistBreakLock->Acquire();
                receptionists[ID].ReceptionistBreakCV->Broadcast(receptionists[ID].ReceptionistBreakLock);
                receptionists[ID].ReceptionistBreakLock->Release();
                
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
    
    t = new Thread("hospital_0");
    t->Fork((VoidFunctionPtr) hospitalManager, 0);
    
    
}

void HospINIT() {
    
    MAX_DOCTORS = 5;
    RECP_MAX = 1;
    MAX_PATIENTS = 3;
    
    INIT(); // initiate threads
}
