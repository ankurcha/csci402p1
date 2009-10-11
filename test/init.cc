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
#include "list.h"
#include <stdlib.h>	// for itoa() call
#include <iostream>
#include "itoa.cc"

using namespace std;

#define BUSY 0
#define FREE 1
#define SLEEPING 2

bool test1active = false;
bool test2active = false;
bool test4active = false;
bool test7active = false;
bool test5active = false;

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
        return -1;
    }
};
char inttochar(int num){
	
	char buf[5];

	
	itoa(num, buf, 10);
	
	return buf;
}

Lockid *testlock = CreateLock("lock_test");
//Lock *testlock = new Lock("TestLock");

    // tokenCounter for assigning tokens to patients
Lock *TokenCounterLock = new Lock("TokenCounterLock");
int TokenCounter;

    // global for all receptionists
Lock* recpLineLock = new Lock("recpLineLock");

    //shared data struct related to a Receptionist
struct Receptionists{
        // receptionist line CV
    Condition *receptionCV;
    int peopleInLine;
    
        // receptionist transactional lock and CV and protected variables
    Lock* transLock;
    Condition *receptionistWaitCV;
    int currentToken;
    
        // receptionist break CV
    Condition *ReceptionistBreakCV;
    
    Receptionists(){
        peopleInLine = 0;
        
        receptionCV = new Condition("receptionCV");
        
        transLock = new Lock("Receptionists.transLock");
        receptionistWaitCV = new Condition("receptionistWaitCV");
        ReceptionistBreakCV = new Condition("ReceptionistBreakCV");
        currentToken = 0;
    }
};

    // list mapping patient tokens to consultFees
Lock* feeListLock = new Lock("feeListLock");
linkedlist* feeList = new linkedlist();

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
    
        //protected by feesPaidLock, but only modified by one thread
    int sales;
    
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

    // hospitalLock protects the count of patients remaining in the hospital
Lock *hospitalLock = new Lock("HospitalLock");
int peopleInHospital = 1;


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
    
        //protected by PaymentLock
    int sales;
    
    PharmacyClerks(){
        patientsInLine= 0;
        state=FREE;
        payment=0;
        fee=(int)(Random())%100;
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
        //CV for doorboys to sleep on
    Condition* doorboyBreakCV;
    
    
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
        doorboyBreakCV = new Condition("Doctor.doorboyBreakCV");
        
        transLock = new Lock("Doctor.transLock");
        transCV = new Condition("Doctor.transCV");
    }
    
    ~Doctor() {
        delete LineLock;
        delete LineCV;
        delete doorboyBreakCV;
        delete transLock;
        delete transCV;
    }
};

    // globals to track the queue of doorboys waiting to service doctors
Lock* doorboyLineLock = new Lock("doorboyLineLock");
Condition* doorboyLineCV = new Condition("doorboyLineCV");
int doorboyLineLength = 0;
    //int wakingDoctorID = 0;
List* wakingDoctorList = new List();

struct DoorBoy {
        //Condition *doorboyBreakCV;
    
    DoorBoy(){
            //doorboyBreakCV = new Condition("doorboyBreakCV");
    }
};

const int MAX_DOCTORS = 10;
const int MIN_DOCTORS = 4;

const int MAX_DOORB = MAX_DOCTORS;
const int MIN_DOORB = MIN_DOCTORS;

const int MAX_PATIENTS = 100;
const int MIN_PATIENTS = 20;

const int RECP_MAX = 5;
const int RECP_MIN = 3;

const int MAX_CLERKS = 5;
const int MIN_CLERKS = 3;

const int MAX_CASHIER = 5;
const int MIN_CASHIER = 3;

const int totalHospMan = 1;


int numDoctors = 0;
int numCashiers = 0;
int numClerks = 0;
int numDoorboys = 0;
int numRecp = 0;
int numPatients = 0;

Receptionists receptionists[RECP_MAX];
DoorBoy doorboys[MAX_DOCTORS];
Doctor doctors[MAX_DOCTORS];
Cashier cashiers[MAX_CASHIER];
PharmacyClerks clerks[MAX_CLERKS];

int test_state = 0;

#include "patient.cc"

void doorboy(int ID){
    int myDoctor = 0;
    
    while (true) {
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(": Alive ",50,ConsoleOutput);
        
            //Get into the doorboyLine till some doctor asks for me
        doorboyLineLock->Acquire();
        
        doorboyLineLength++;
        Write("DB_",15,1);
        Write(itoa(ID),10,1);
        Write(": Waiting for some doctor to wake me up.\n",50,1) ;
        doorboyLineCV->Wait(doorboyLineLock);
        doorboyLineLength--;
        
            //Some doctor woke me up, lets check who
            //myDoctor =  wakingDoctorID;
        if(wakingDoctorList->IsEmpty()) {
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(": ERROR: Waking doctor list is empty!\n",50,1);
            continue;
        }
        myDoctor = (int) wakingDoctorList->Remove();
        if(test2active==true)
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(":TEST2: Servicing D_",50,1);
        Write(itoa(myDoctor),50,1);
        Write("\n",1,1);
        else
       	Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(":Servicing D_",50,1);
        Write(itoa(myDoctor),50,1);
        Write("\n",1,1);
           
        doorboyLineLock->Release();
        
            // Inform the doctor that I have arrived, and wait for him to take 
            //  a break, if he so chooses
        doctors[myDoctor].transLock->Acquire();
        doctors[myDoctor].transCV->Signal(doctors[myDoctor].transLock);
        doctors[myDoctor].transCV->Wait(doctors[myDoctor].transLock);
        
            ///// PATIENT LINE /////
            //Acquire the lock to get the state of the line and take decision
        doctors[myDoctor].LineLock->Acquire();
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(": Checking for Patients\n",50,1);
        
            //while there is noone in line
        bool doorboyBreak = false;
        while(doctors[myDoctor].peopleInLine <= 0) { 
            doorboyBreak = true;
                //I will be woken up by the manager only!!
            
                // prefix for test conditions
            if(myDoctor == 0 && test_state == 8)
            	Write("T8: ",50,1);
                //cout << "T8: ";
            if(test_state == 11)
            	Write("T11: ",50,1);
              //  cout << "T11: ";
                // midfix for test 2
            if(test2active==true) {
            	Write("DB_",10,1);
              Write(itoa(ID),10,1);
               Write(":TEST2: Yawn!!...ZZZZzzzzz....\n",50,1);
            } else {
            	Write("DB_",10,1);
              Write(itoa(ID),10,1);
               Write(": Yawn!!...ZZZZzzzzz....\n",50,1);
                
            }
            doctors[myDoctor].doorboyBreakCV->Wait(doctors[myDoctor].LineLock);
                // I got woken up, time to go back to work - by now there are 
                //  people dying on the floor!
        }
        if(doorboyBreak) {
                // prefix for test 8 condition
            if(myDoctor == 0 && test_state == 8) {
            	Write("T8: \n",10,1);
            
            
            }
            Write("DB_",10,1);
            Write(itoa(ID),10,1);
            Write(": Woken up!\n",50,1);
        }
        
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(": Found ",20,1);
        Write(itoa(doctors[myDoctor].peopleInLine),20,1);
       Write(" patients waiting in line for D_ ",50,1);
        Write(itoa(myDoctor),10,1);
             
        
            //Now wake the patient up to go to the doctor
        Write("DB_",10,1);
        Write(itoa(ID),10,1);
        Write(":Tell patient to go to doctor D_",50,1);
        Write(itoa(myDoctor),10,1);
        
        doctors[myDoctor].LineCV->Signal(doctors[myDoctor].LineLock);
        
            //My job with the patients and the doctor is done
            //I can go back on the doorboyLine
        doctors[myDoctor].transLock->Release();
        doctors[myDoctor].LineLock->Release();
        
    }//End of while
    Write("DB_",10,1);
    Write(itoa(ID),10,1);
    Write(":Dying...AAAaaaahhhhhhhhh!!\n",50,1);
   
    
}

void doctor(int ID){
    int waitingtime = 10000;
    while(true) {
            // acquire a doorboy
        Write("D_",50,1);
        Write(itoa(ID),10,1);
        Write(": Alive!!\n",50,1);
        doorboyLineLock->Acquire();
        
            // assure that there is a doorboy in line
        while(doorboyLineLength <= 0) {
            if(waitingtime % 100 == 0){
            	Write("D_",50,1);
              Write(itoa(ID),10,1);
              Write(": Doctor could not find a doorboy waittime: ",50,1);
              Write(itoa(waitingtime),50,1);
              Write("\n",10,1);
            }
            
            doorboyLineLock->Release();
            currentThread->Yield();
            waitingtime--;
            doorboyLineLock->Acquire();
            if(waitingtime <= 0){
                Write("Waited for a long time with no Doorboys, exiting...\n",50,1);
                return;
            }
        }
        
            // pull the next doorboy off the line
            Write("D_",50,1);
            Write(itoa(ID),10,1);
            Write(":Signaling doorboy!\n",50,1);
            
        
            //wakingDoctorID = ID;
        wakingDoctorList->Append( (void*) ID);
        doorboyLineCV->Signal(doorboyLineLock);
        
            // acquire the transaction lock and wait for the doorboy to arrive
        doctors[ID].transLock->Acquire();
        doorboyLineLock->Release();
        
            //////  DOORBOY INTERACTION  //////
        doctors[ID].transCV->Wait(doctors[ID].transLock);
        
        bool doctorBreak = false;
            // go on break if so inclined
        
        if(test7active==true)
       	{
       		int numYields = 35;
       		Write("D_",50,1);
          Write(itoa(ID),10,1);
          Write(" :TEST7: Going on break for ",50,1);
          Write(itoa(numYields),50,1);
          Write(" cycles!\n",50,1);
            for(int i=0; i < numYields; ++i) {
                currentThread->Yield();
            }
       		
       	}
        else
            if(Random() % 100 > 49) { // go on break
                doctorBreak = true;
                    // 5-15 yields
                int numYields = 5 + (Random() % 11);
                
                    // provide a handle for test 8, only uses doctor 0
                if(ID == 0 && test_state == 8 ) { 
                	Write("T8: ",50,1);
                    
                }
                
                Write("D_",50,1);
                Write(itoa(ID),10,1);
                Write(": Going on break for ",50,1);
                Write(itoa(numYields),50,1);
                Write(" cycles!\n",50,1);
                
                for(int i=0; i < numYields; ++i) {
                    currentThread->Yield();
                }
            }
        
        
        
            // provide a handle for test 8, only uses doctor 0
        if(ID == 0 && test_state == 8 && doctorBreak) { 
        	Write("T8: ",50,1);
            
        }
        if(doctorBreak) 
        	Write("D_",50,1);
        	 Write(itoa(ID),10,1);
        	 
           Write(": Back from Break\n",50,1);
        
            // inform the doorboy that I am ready for a patient
        
        if(test7active==true)
       	{
       		Write("D_",50,1);
        	 Write(itoa(ID),10,1);
       		Write(":TEST7: Back from Break,Signalling patient to come in.\n",50,1);
       		
       	}
       	else
       		Write("D_",50,1);
        	 Write(itoa(ID),10,1);
       		Write(": Back from Break,Signalling patient to come in.\n",50,1);
       		
            
        
        doctors[ID].transCV->Signal(doctors[ID].transLock);
        Write("D_",50,1);
        Write(itoa(ID),10,1);
        Write(": Waiting for patient....\n",50,1);
        
            //////  PATIENT INTERACTION  //////
            // and wait for that patient to arrive
        doctors[ID].transCV->Wait(doctors[ID].transLock);
        
            // consult: 10-20 yields
        Write("D_",50,1);
        Write(itoa(ID),10,1);
        Write(": Now Consulting patient\n",50,1);
        
        int numYields = 10 + (Random() % 11);
        for(int i=0; i < numYields; ++i) {
            currentThread->Yield();  // I see ... mm hmm ... does it hurt here? ...
        }
        
            // give prescription to patient
        doctors[ID].prescription = Random() % 100;
        
            // put consultation fees into the data structure for the cashier ($50-$250)
        cout<<"D_"<<ID<<": Telling fee to cashiers\n";
        int consultFee = 50 + (Random() % 201);
        feeListLock->Acquire();
        feeList->append(doctors[ID].patientToken, consultFee);
        feeListLock->Release();
        
            // pass the prescription to the patient and wait for them to leave
        Write("D_",50,1);
        Write(itoa(ID),10,1);
        Write(": Waiting for the patient to leave\n",50,1);
        
        doctors[ID].transCV->Signal(doctors[ID].transLock);
        doctors[ID].transCV->Wait(doctors[ID].transLock);
        
            // done, the patient has left
        doctors[ID].transLock->Release();
        Write("D_",50,1);
        Write(itoa(ID),10,1);
        Write(": I'm ready for another one\n",50,1);
        
        
    } //end while
}

void receptionist(int ID){
    while (true) {
    	Write("R_",50,1);
      Write(itoa(ID),10,1);
      Write(": Alive!\n",50,1);
        
        recpLineLock->Acquire();
        if (receptionists[ID].peopleInLine > 0) {
                //Wake one waiting patient up
            receptionists[ID].receptionCV->Signal(recpLineLock);
        } else {
                //My Line is empty
            DEBUG('t',"No Patients, going on break...");
            
                // prefix for test condition
            if(test_state == 11)
            Write( "T11: ",10,1);
            Write("R_",50,1);
            Write(itoa(ID),10,1);
            Write(":Going to sleep\n",50,1);
           
            receptionists[ID].ReceptionistBreakCV->Wait(recpLineLock);
            recpLineLock->Release();
                //HospitalManager kicked my ass for sleeping on the job!!
                //Loop back!!
            continue;
        }
        
        receptionists[ID].transLock->Acquire();
        recpLineLock->Release();
        
            //Genetate token for the patient
        TokenCounterLock->Acquire();
        Write("R_",50,1);
        Write(itoa(ID),10,1);
        Write(": Generating Token...\n",50,1);
        
        
        receptionists[ID].currentToken = ++TokenCounter;
        TokenCounterLock->Release();
        
            //Sleep till you get Acknowledgement
        Write("R_",50,1);
        Write(itoa(ID),10,1);
        Write(":  Waiting for Patient to pick up token...\n",50,1);
        
        receptionists[ID].receptionistWaitCV->Wait(receptionists[ID].transLock);
        
            //Patient successfully got the token, go back to work: Loop again
        Write("R_",50,1);
        Write(itoa(ID),10,1);
        Write(": Patient got token, Continue to next Patient\n",50,1);
        
        receptionists[ID].transLock->Release();
    }
    
}

void cashier(int ID) {
	
	  Write("Cash_",50,1);
        Write(itoa(ID),10,1);
        Write(":  Alive!!\n",50,1);
	  
        while(true) {
        cashierLineLock->Acquire();
        
        if(cashiers[ID].lineLength > 0) { // someone in line
                                          //signal person on top
        Write("Cash_",50,1);
        Write(itoa(ID),10,1);
        Write(":  someone in my line...\n",50,1);
                                          
            
            cashiers[ID].lineCV->Signal(cashierLineLock);
        } else { // noone in line
                 // go on break
            
                // prefix for test condition
            if(test_state == 11)
            	Write("T11: ",50,1);
        Write("Cash_",50,1);
        Write(itoa(ID),10,1);
        Write(":  No one in line... going on break\n",50,1);
            
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
        
            // lookup value for cashiers[ID].patToken in the token table
        feeListLock->Acquire();
        cashiers[ID].fee = feeList->getValue(cashiers[ID].patToken);
        feeListLock->Release();
            // tell patient the fee
        
        cashiers[ID].transCV->Signal(cashiers[ID].transLock);
            // wait for payment
        cashiers[ID].transCV->Wait(cashiers[ID].transLock);
        
            // add this payment to our total collected
        feesPaidLock->Acquire();
        feesPaid += cashiers[ID].payment;
        cashiers[ID].sales += cashiers[ID].payment;
        feesPaidLock->Release();
        if(cashiers[ID].payment < cashiers[ID].fee) {
        	Write("ERROR: call security, that patient didin't pay!",50,1);
            
        }        
        
        cashiers[ID].transLock->Release();
    }
}


void clerk(int ID){
    while(true){
        ClerkLinesLock->Acquire();
        
        if(clerks[ID].patientsInLine > 0) { // someone in line
                                            //signal the first person
            clerks[ID].ClerkCV->Signal(ClerkLinesLock);
        } else { // noone in line
                 // go on break
            
                // prefix for test condition
            if(test_state == 11)
            	Write("T11: ",50,1);
                
            Write("CL_",10,1);
            Write(itoa(ID),50,1);
            Write(": Going on break\n",50,1);
            
            
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
        
            Write("CL_",10,1);
            Write(itoa(ID),50,1);
            Write(": gave Medicines!\n",50,1);
        
        
        clerks[ID].ClerkTransCV->Signal(clerks[ID].ClerkTransLock);
            // wait for payment
        clerks[ID].ClerkTransCV->Wait(clerks[ID].ClerkTransLock);
            //Collect payment
            
            Write("CL_",50,1);
            Write(itoa(ID),50,1);
            Write(": The cost for the medicines are:",50,1);
            Write(itoa(clerks[ID].fee),50,1);
            Write(" Dollars\n",50,1);
        
        
            // add this payment to our total collected
        PaymentLock->Acquire();
        totalsales += clerks[ID].payment;
        clerks[ID].sales += clerks[ID].payment;
        PaymentLock->Release();
        
        
        clerks[ID].ClerkTransLock->Release();
    }
}


void hospitalManager(int ID){
	  Write("H_",50,1);
	  Write(itoa(ID),50,1);
	  Write(": Alive\n",50,1);
    
    int sleeptime = Random() % 30000;
    int test5cycles = 1;
    while (true) {
        if (test_state == 51 || test_state == 52 || test_state == 53) {
                //The patients will always be there in the system.
                //For test purposes, lets assume the simulation to be
                //complete after 100 cycles.
            if (test5cycles > 0) {
                test5cycles--;
            }else {
                return;
            }   
        }
        hospitalLock->Acquire();
        if (peopleInHospital <= 0) {
        	Write("H_",50,1);
	  Write(itoa(ID),50,1);
	  Write(":  No one to service, Killing myself!!!\n",50,1);
           
            return;
        }
        hospitalLock->Release();
        
        sleeptime = Random() % 30000;
            //Sleep for some random amount of time
       Write("H_",50,1);
	  Write(itoa(ID),50,1);
	  Write(":  Sleeping for",50,1);
	  Write(itoa(sleeptime),50,1);
	  Write(" cycles\n",50,1);
            
       
        do{
            currentThread->Yield();
            sleeptime--;
        }while (sleeptime > 0);
            //I am on rounds now, Time to kick some ass
        Write("H_",50,1);
	      Write(itoa(ID),50,1);
	      Write(": Going on rounds\n",50,1);
        
        
        
        
            //1. Check on the Receptionists
        Write("H_",50,1);
	      Write(itoa(ID),50,1);
	      Write(": Checking receptionists\n",50,1);
        
        int patientsWaiting=0;
        for (int j=0; j<numRecp; j++) {
            patientsWaiting += receptionists[j].peopleInLine;
        }
        
        if (patientsWaiting > 1) {
            for (int j=0; j<numRecp; j++) {
                recpLineLock->Acquire();
                receptionists[j].ReceptionistBreakCV->Signal(recpLineLock);
                recpLineLock->Release();
            }
        }
            //2. Query Cashiers
            
        Write("H_",50,1);
	      Write(itoa(ID),50,1);
	      Write(": Checking cashiers\n",50,1);
        
        for (int i=0; i<numCashiers; i++) {//Check for waiting patients
            if (cashiers[i].lineLength > 0 ) {
        
        Write("H_",50,1);
	      Write(itoa(ID),50,1);
	      Write(": Found",50,1);
	      Write(itoa(cashiers[i].lineLength),50,1);
	      Write(" patients waiting for C_",50,1);
        Write(itoa(i),50,1);
        Write("  -> Signal Cashier\n",50,1);	      
                
                    //Wake up this receptionist up
                cashierLineLock->Acquire();
                cashiers[i].breakCV->Broadcast(cashierLineLock);
                cashierLineLock->Release();
                
            }
        }
        
            //Query cashiers for total sales
        feesPaidLock->Acquire();
        Write(" T10: Total fees collected by cashiers:",50,1);
        Write(itoa(feesPaid),50,1);
        
        if( test_state == 10 ) {
                // this is a test for race conditions, so we can't have any:
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            int sum = 0;
            for (int i=0; i<numCashiers; i++) {
            	Write(" T10: cashier",50,1);
            	Write(itoa(i),50,1);
            	Write(" :",50,1);
            	Write(itoa(cashiers[i].sales),50,1);
              Write("\n",50,1);  
              
                sum += cashiers[i].sales;
            }
            Write("T10: TOTAL:",50,1);
            Write(itoa(sum),50,1);
            
            
            
                // sum just printed should match feesPaid, printed earlier
            (void) interrupt->SetLevel(oldLevel);
        }
        feesPaidLock->Release();
        
        
            //3. Query pharmacy
        Write("H_",50,1);
        Write(itoa(ID),50,1);
        Write(":Checking clerks\n",50,1);
        
        
        for (int i=0; i<numClerks; i++) {//Check for waiting patients
            if (clerks[i].patientsInLine > 0 ) {
            	 Write("H_",50,1);
            	 Write(itoa(ID),50,1);
               Write(": found CL_",50,1);
               Write(itoa(i),50,1);
               Write(": sleeping and ",50,1);
               Write(itoa(clerks[i].patientsInLine),50,1);
               Write("waiting -> Signaling Clerk\n",50,1);
               
                //Wake up this clerk up
                ClerkLinesLock->Acquire();
                clerks[i].ClerkBreakCV->Signal(ClerkLinesLock);
                ClerkLinesLock->Release();
            }
        }
        
            //Query clerks for total sales
        PaymentLock->Acquire();
               Write("H_",50,1);
            	 Write(itoa(ID),50,1);
               Write("T10: Total amount collected by clerks: ",50,1);
               Write(itoa(totalsales),50,1);
               Write("\n",50,1);
        
        
        if( test_state == 10 ) {
                // this is a test for race conditions, so we can't have any:
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            int sum = 0;
            for (int i=0; i<numClerks; i++) {
            	
            	 Write("T10: clerk ",50,1);
            	 Write(itoa(i),50,1);
               Write(" : ",50,1);
               Write(itoa(clerks[i].sales),50,1);
               Write("\n",50,1);
               
                sum += clerks[i].sales;
            }
            Write("T10: TOTAL: ",50,1);
            Write(itoa(sum),50,1);
            
                // sum just printed should match feesPaid, printed earlier
            (void) interrupt->SetLevel(oldLevel);
        }
        PaymentLock->Release();
        
        currentThread->Yield();
        
            //Check on the doorboys
               Write("H_",50,1);
            	 Write(itoa(ID),50,1);
            	 Write(": Checking doorboys\n",50,1);
        
        for (int i=0; i<numDoctors; i++) {//Check for waiting patients
            if (doctors[i].peopleInLine > 0 ) {
            	  
            	 Write("H_",50,1);
            	 Write(itoa(ID),50,1);
               Write(": found ",50,1);
               Write(itoa(doctors[i].peopleInLine),50,1);
               Write(": people in doctor ",50,1);
               Write(itoa(i),50,1);
               Write("'s line -> Signal Doorboy\n",50,1);
               
                
                doctors[i].LineLock->Acquire();
                doctors[i].doorboyBreakCV->Broadcast(doctors[i].LineLock);
                doctors[i].LineLock->Release();
            }
        }        
    }
}

void HospINIT(int testmode = 0) {
    
        // set a global so everyone will know the test mode
    test_state = testmode;
    
    if(testmode != 1 && testmode != 51 && testmode != 52 && testmode != 53 ){
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        Thread *t;   
        
            //cout << "Simulation startup\n\n";
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
               Write("Creating ",50,1);
            	 Write(itoa(numCashiers),50,1);
               Write("Cashiers\n",50,1);
        
        
        for(i=0;i<numCashiers;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            Write("Creating ",50,1);
            Write(itoa(numDoorboys),50,1);
            Write(" Doorboys\n",50,1);
            
            for(i=0;i<numDoorboys;i++)
            {
                
                t=new Thread(temp);
                t->Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            Write("Bypassing Doorboy Creation\n",50,1);
            
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            Write("Creating ",50,1);
            Write(itoa(numClerks),50,1);
            Write(" Clerks\n",50,1);
        
        
        for(i=0;i<numClerks;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            
            Write("Creating ",50,1);
            Write(itoa(numDoctors),50,1);
            Write(" Doctors\n",50,1);
        
        for(i=0;i<numDoctors;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        hospitalLock->Acquire();
        peopleInHospital = numPatients;
        hospitalLock->Release();    
        
        Write("Creating ",50,1);
            Write(itoa(numPatients),50,1);
            Write(" Patients\n",50,1);
            
        
        for(i=0;i<numPatients;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager
        Write("Creating 1 Hospital Manager\n ",50,1);    
        
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   
        
        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
            Write("Creating ",50,1);
            Write(itoa(numRecp),50,1);
            Write(" Receptionists\n",50,1);
            
        
        for(i=0; i<numRecp; i++)
        {
            t = new Thread(temp);
            t->Fork((VoidFunctionPtr) receptionist, i);
        }
        
        
    }else if (testmode == 51) {
        
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        Thread *t;   
        
        
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            Write("Creating ",50,1);
            Write(itoa(numCashiers),50,1);
            Write(" Cashiers\n",50,1);
            
        
        for(i=0;i<numCashiers;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            Write("Creating ",50,1);
            Write(itoa(numDoorboys),50,1);
            Write(" Doorboys\n",50,1);
            
            
            for(i=0;i<numDoorboys;i++)
            {
                
                t=new Thread(temp);
                t->Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            Write(" Bypassing Doorboy Creation\n",50,1);
            
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
        
            Write("Creating ",50,1);
            Write(itoa(numClerks),50,1);
            Write(" Clerks\n",50,1);

       
        for(i=0;i<numClerks;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            Write("Creating ",50,1);
            Write(itoa(numDoctors),50,1);
            Write(" Doctors\n",50,1);
        
        for(i=0;i<numDoctors;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        hospitalLock->Acquire();
        peopleInHospital = numPatients;
        hospitalLock->Release();    
        
        Write("Creating ",50,1);
            Write(itoa(numPatients),50,1);
            Write(" Patients\n",50,1);
        
        for(i=0;i<numPatients;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager
            
        Write("Creating 1 Hospital Manager\n ",50,1); 
        
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   
        
        
        
        
            //2. No Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
    }else if (testmode == 52) {
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        Thread *t;   
        
        
        
            //3. No Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
        
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            
            Write("Creating ",50,1);
            Write(itoa(numDoorboys),50,1);
            Write(" Doorboys\n",50,1);

            
            for(i=0;i<numDoorboys;i++)
            {
                
                t=new Thread(temp);
                t->Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            Write(" Bypassing Doorboy Creation\n",50,1);
            
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            Write("Creating ",50,1);
            Write(itoa(numClerks),50,1);
            Write(" Clerks\n",50,1);
        
        for(i=0;i<numClerks;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            Write("Creating ",50,1);
            Write(itoa(numDoctors),50,1);
            Write(" Doctors\n",50,1);

       
        for(i=0;i<numDoctors;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        hospitalLock->Acquire();
        peopleInHospital = numPatients;
        hospitalLock->Release();    
        
            Write("Creating ",50,1);
            Write(itoa(numPatients),50,1);
            Write(" Patients\n",50,1);
        
        
        for(i=0;i<numPatients;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager
            
            
        Write("Creating 1 Hospital Manager\n ",50,1);     
        
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   
        
        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
            Write("Creating ",50,1);
            Write(itoa(numRecp),50,1);
            Write(" Receptionists\n",50,1);
        
        
        for(i=0; i<numRecp; i++)
        {
            t = new Thread(temp);
            t->Fork((VoidFunctionPtr) receptionist, i);
        }
        
        
    }else if (testmode == 53) {
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        Thread *t;   
        
        
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            
            Write("Creating ",50,1);
            Write(itoa(numCashiers),50,1);
            Write(" Cashiers\n",50,1);

        
        for(i=0;i<numCashiers;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            
            Write("Creating ",50,1);
            Write(itoa(numDoorboys),50,1);
            Write(" Doorboys\n",50,1);

            
            for(i=0;i<numDoorboys;i++)
            {
                
                t=new Thread(temp);
                t->Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            
            Write(" Bypassing Doorboy Creation\n",50,1);
            
            
        }
        
        
            //5. No Pharmacy clerks
        numClerks = (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;        
        
            //1. Doctors
            Write("Creating ",50,1);
            Write(itoa(numDoctors),50,1);
            Write(" Doctors\n",50,1);
            
           
       
        for(i=0;i<numDoctors;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        hospitalLock->Acquire();
        peopleInHospital = numPatients;
        hospitalLock->Release();    
        
        
            Write("Creating ",50,1);
            Write(itoa(numPatients),50,1);
            Write(" Patients\n",50,1);

        
        for(i=0;i<numPatients;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager
            
        Write("Creating 1 Hospital Manager\n ",50,1);      
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   
        
        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
        Write("Creating ",50,1);
        Write(itoa(numRecp),50,1);
        Write(" Receptionists\n",50,1);
        
        
        for(i=0; i<numRecp; i++)
        {
            t = new Thread(temp);
            t->Fork((VoidFunctionPtr) receptionist, i);
        }
    }else if (testmode == 2) {
    }
}


int test1(){
    test1active = true;
    HospINIT();
    return 0;
}

int test2(){
	test2active=true;
	HospINIT();
	return 0;
}

int test4(){
    test4active = true;
        //start the process normally
    HospINIT();
    return 0;
    
}


int test7(){
    test7active = true;
    HospINIT();
    return 0;
}
