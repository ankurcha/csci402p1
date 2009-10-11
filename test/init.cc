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
 




#include "syscall.h"
#include "list.h"
#include "itoa.cc"
#include "print.cc"
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



LockId testlock = CreateLock("TestLock");

    // tokenCounter for assigning tokens to patients
LockId TokenCounterLock = CreateLock("TokenCounterLock");
int TokenCounter;

    // global for all receptionists
LockId recpLineLock = CreateLock("recpLineLock");

    //shared data struct related to a Receptionist
struct Receptionists{
        // receptionist line CV
    CVId receptionCV;
    int peopleInLine;
    
        // receptionist transactional lock and CV and protected variables
    LockId transLock;
    CVId receptionistWaitCV;
    int currentToken;
    
        // receptionist break CV
    CVId ReceptionistBreakCV;
    
    Receptionists(){
        peopleInLine = 0;
        
        receptionCV = CreateCondition("receptionCV");
        
        transLock = CreateLock("Receptionists.transLock");
        receptionistWaitCV = CreateCondition("receptionistWaitCV");
        ReceptionistBreakCV = CreateCondition("ReceptionistBreakCV");
        currentToken = 0;
    }
};

    // list mapping patient tokens to consultFees
LockId feeListLock = CreateLock("feeListLock");
linkedlist* feeList = new linkedlist();

    // global for all cashiers
LockId cashierLineLock = CreateLock("cashierLineLock");
LockId feesPaidLock = CreateLock("feesPaidLock");
int feesPaid = 0;

    // shared data struct related to a Cashier
struct Cashier {
        // line CV and length
    int lineLength;
    CVId lineCV;
    
        // transaction lock, CV, and variables protected by the former
    LockId transLock;
    CVId transCV;
    int patToken;
    int fee;
    int payment;
    
        //protected by feesPaidLock, but only modified by one thread
    int sales;
    
        // cashier's CV for going on break
    CVId breakCV;
    
    Cashier() {
        lineLength = 0;
        patToken = 0;
        fee = 0;
        payment = 0;
        
        lineCV = CreateCondition("Cashier.lineCV");
        transLock = CreateLock("Cashier.transLock");
        transCV = CreateCondition("Cashier.transCV");
        breakCV = CreateCondition("Cashier.breakCV");
    }
    
    ~Cashier() {
        destroyCondition( lineCV );
        destroyLock( transLock );
        destroyCondition( transCV );
        destroyCondition( breakCV );
    }
};

LockId ClerkLinesLock= CreateLock("ClerkLineLock");
LockId PaymentLock= CreateLock("PaymentLock");
int totalsales=0;

    // hospitalLock protects the count of patients remaining in the hospital
LockId hospitalLock = CreateLock("HospitalLock");
int peopleInHospital = 1;


struct PharmacyClerks{
    int patientsInLine;
    int state;
    int payment;
    int fee;
    int patPrescription;
    CVId ClerkCV;
    
    CVId ClerkBreakCV;
    LockId ClerkTransLock;
    CVId ClerkTransCV;
    
        //protected by PaymentLock
    int sales;
    
    PharmacyClerks(){
        patientsInLine= 0;
        state=FREE;
        payment=0;
        fee=(int)(Random())%100;
        patPrescription=0;
        
        ClerkCV = CreateCondition("ClerkCV");
        ClerkBreakCV = CreateCondition("ClerkBreakCV");
        ClerkTransLock = CreateLock("ClerkTransLock");
        ClerkTransCV = CreateCondition("ClerkTransCV");
    }  
};

struct Doctor {
        // line lock and CV and protected variables
    LockId LineLock;
    CVId LineCV;
    int peopleInLine;
        //CV for doorboys to sleep on
    CVId doorboyBreakCV;
    
    
        //transaction lock and CV and variables protected
    LockId transLock;
    CVId transCV;
    int prescription;
    int patientToken;
    
    Doctor() {
        prescription = -1;
        patientToken = -1;
        
        peopleInLine = 0;
        LineLock = CreateLock("LineLock");
        LineCV = CreateCondition("LineCV");
        doorboyBreakCV = CreateCondition("Doctor.doorboyBreakCV");
        
        transLock = CreateLock("Doctor.transLock");
        transCV = CreateCondition("Doctor.transCV");
    }
    
    ~Doctor() {
        destroyLock(LineLock);
        destroyCondition(LineCV);
        destroyCondition(doorboyBreakCV);
        destroyLock(transLock);
        destroyLock(transCV);
    }
};

    // globals to track the queue of doorboys waiting to service doctors
LockId doorboyLineLock = CreateLock("doorboyLineLock");
CVId doorboyLineCV = CreateCondition("doorboyLineCV");
int doorboyLineLength = 0;
    //int wakingDoctorID = 0;
List* wakingDoctorList = new List();

struct DoorBoy {
    DoorBoy(){
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
        print("DB_");
        print(itoa(ID));
        print(": Alive ");
        
            //Get into the doorboyLine till some doctor asks for me
        Acquire(doorboyLineLock)
        
        doorboyLineLength++;

        print("DB_");
        print(itoa(ID));
        print(": Waiting for some doctor to wake me up.") ;
        Write("\n");
        doorboyLineCV->Wait(doorboyLineLock);

        doorboyLineLength--;
        
            //Some doctor woke me up, lets check who
            //myDoctor =  wakingDoctorID;
        if(wakingDoctorList->IsEmpty()) {
        print("DB_");
        print(itoa(ID));
        print(": ERROR: Waking doctor list is empty!");
        Write("\n");
            continue;
        }
        myDoctor = (int) wakingDoctorList->Remove();
        if(test2active==true)
        print("DB_");
        print(itoa(ID));
        print(":TEST2: Servicing D_");
        print(itoa(myDoctor));
        Write("\n");
        else

       	print("DB_");
        print(itoa(ID));
        print(":Servicing D_");
        print(itoa(myDoctor));
        Write("\n");
           
        doorboyLineLock->Release();

        
            // Inform the doctor that I have arrived, and wait for him to take 
            //  a break, if he so chooses
        Acquire(doctors[myDoctor].transLock);
        Signal(doctors[myDoctor].transCV,doctors[myDoctor].transLock);
        Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
        
            ///// PATIENT LINE /////
            //Acquire the lock to get the state of the line and take decision

        doctors[myDoctor].LineLock->Acquire();
        print("DB_");
        print(itoa(ID));
        print(": Checking for Patients");
        Write("\n");
        
            //while there is noone in line
        bool doorboyBreak = false;
        while(doctors[myDoctor].peopleInLine <= 0) { 
            doorboyBreak = true;
                //I will be woken up by the manager only!!
            
                // prefix for test conditions
            if(myDoctor == 0 && test_state == 8)
            	print("T8: ");
                
            if(test_state == 11)
            	print("T11: ");
              if(test2active==true) {
            	print("DB_");
              print(itoa(ID));
               print(":TEST2: Yawn!!...ZZZZzzzzz....");
               Write("\n");
            } else {
            	print("DB_");
              print(itoa(ID),);
               print(": Yawn!!...ZZZZzzzzz....");
               Write("\n");
                
            }
            Wait(doctors[myDoctor].doorboyBreakCV, doctors[myDoctor].LineLock);
                // I got woken up, time to go back to work - by now there are 
                //  people dying on the floor!
        }
        if(doorboyBreak) {
                // prefix for test 8 condition
            if(myDoctor == 0 && test_state == 8) {
            	print("T8: ");
            	Write("\n");
            
            
            }
            print("DB_");
            print(itoa(ID));
            print(": Woken up!");
            Write("\n");
        }
        
        print("DB_");
        print(itoa(ID));
        print(": Found ");
        print(itoa(doctors[myDoctor].peopleInLine));
       print(" patients waiting in line for D_ ");
        print(itoa(myDoctor));
        Write("\n");
             
        
            //Now wake the patient up to go to the doctor

        print("DB_");
        print(itoa(ID));
        print(":Tell patient to go to doctor D_");
        print(itoa(myDoctor));
        Write("\n");
        
        doctors[myDoctor].LineCV->Signal(doctors[myDoctor].LineLock);

        
            //My job with the patients and the doctor is done
            //I can go back on the doorboyLine
        Release(doctors[myDoctor].transLock);
        Release(doctors[myDoctor].LineLock);
        
    }//End of while
    print("DB_");
    print(itoa(ID));
    print(":Dying...AAAaaaahhhhhhhhh!!");
    Write("\n");
   
    
}

void doctor(int ID){
    int waitingtime = 10000;
    while(true) {
            // acquire a doorboy

        print("D_");
        print(itoa(ID));
        print(": Alive!!");
        Write("\n");
        doorboyLineLock->Acquire();

        
            // assure that there is a doorboy in line
        while(doorboyLineLength <= 0) {
            if(waitingtime % 100 == 0){
            	print("D_");
              print(itoa(ID));
              print(": Doctor could not find a doorboy waittime: ");
              print(itoa(waitingtime));
              Write("\n");
           
            }
            
            Release(doorboyLineLock);
            currentThread->Yield();
            waitingtime--;
            Acquire(doorboyLineLock);
            if(waitingtime <= 0){
                print("Waited for a long time with no Doorboys, exiting...");
                Write("\n");
                return;
            }
        }
        
            // pull the next doorboy off the line
            print("D_");
            print(itoa(ID));
            print(":Signaling doorboy!");
            Write("\n");
            
        
            //wakingDoctorID = ID;
        wakingDoctorList->Append( (void*) ID);
        Signal(doorboyLineCV,doorboyLineLock);
        
            // acquire the transaction lock and wait for the doorboy to arrive
        Acquire(doctors[ID].transLock);
        Release(doorboyLineLock);
        
            //////  DOORBOY INTERACTION  //////
        Wait(doctors[ID].transCV, doctors[ID].transLock);
        
        bool doctorBreak = false;
            // go on break if so inclined
        
        if(test7active==true)
       	{
       		int numYields = 35;
       		print("D_");
          print(itoa(ID));
          print(" :TEST7: Going on break for ");
          print(itoa(numYields));
          print(" cycles!");
          Write("\n");
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
                	print("T8: ");
                    
                }
                
                print("D_");
                print(itoa(ID));
                print(": Going on break for ");
                print(itoa(numYields));
                print(" cycles!");
                Write("\n");
                
                for(int i=0; i < numYields; ++i) {
                    currentThread->Yield();
                }
            }
        
        
        
            // provide a handle for test 8, only uses doctor 0
        if(ID == 0 && test_state == 8 && doctorBreak) { 
        	print("T8: ");
            
        }
        if(doctorBreak) 
        	print("D_");
        	 print(itoa(ID));
        	 
           print(": Back from Break");
           Write("\n");
        
            // inform the doorboy that I am ready for a patient
        
        if(test7active==true)
       	{
       		print("D_");
        	 print(itoa(ID));
       		print(":TEST7: Back from Break,Signalling patient to come in.");
       		Write("\n");
       		
       	}
       	else
       		print("D_");
        	 print(itoa(ID));
       		print(": Back from Break,Signalling patient to come in.");
       		Write("\n");
       		
            
        

        doctors[ID].transCV->Signal(doctors[ID].transLock);
        print("D_");
        print(itoa(ID));
        print(": Waiting for patient....");
        Write("\n");
        

            //////  PATIENT INTERACTION  //////
            // and wait for that patient to arrive
        Wait(doctors[ID].transCV, doctors[ID].transLock);
        
            // consult: 10-20 yields
        print("D_");
        print(itoa(ID));
        print(": Now Consulting patient");
        Write("\n");
        int numYields = 10 + (Random() % 11);
        for(int i=0; i < numYields; ++i) {
            currentThread->Yield();  // I see ... mm hmm ... does it hurt here? ...
        }
        
            // give prescription to patient
        doctors[ID].prescription = Random() % 100;
        
            // put consultation fees into the data structure for the cashier ($50-$250)
        print("D_");
        print(itoa(ID));
        print(": Telling fee to cashiers");
        Write("\n");
        
        int consultFee = 50 + (Random() % 201);
        Acquire(feeListLock-);
        feeList->append(doctors[ID].patientToken, consultFee);
        Release(feeListLock);
        
            // pass the prescription to the patient and wait for them to leave

        print("D_");
        print(itoa(ID));
        print(": Waiting for the patient to leave");
        Write("\n");
        
        doctors[ID].transCV->Signal(doctors[ID].transLock);
        doctors[ID].transCV->Wait(doctors[ID].transLock);
        
            // done, the patient has left
        doctors[ID].transLock->Release();
        print("D_");
        print(itoa(ID));
        print(": I'm ready for another one");
        Write("\n");

        
    } //end while
}

void receptionist(int ID){
    while (true) {

    	print("R_");
      print(itoa(ID));
      print(": Alive!");
      Write("\n");
        recpLineLock->Acquire();

        if (receptionists[ID].peopleInLine > 0) {
                //Wake one waiting patient up
            Signal(receptionists[ID].receptionCV, recpLineLock);
        } else {
                //My Line is empty
            DEBUG('t',"No Patients, going on break...");
            
                // prefix for test condition
            if(test_state == 11)

            print( "T11: ");
            print("R_");
            print(itoa(ID));
            print(":Going to sleep");
            Write("\n");
            receptionists[ID].ReceptionistBreakCV->Wait(recpLineLock);
            recpLineLock->Release();

                //HospitalManager kicked my ass for sleeping on the job!!
                //Loop back!!
            continue;
        }
        
        Acquire(receptionists[ID].transLock);
        Release(recpLineLock);
        
            //Genetate token for the patient

        TokenCounterLock->Acquire();
        print("R_");
        print(itoa(ID));
        print(": Generating Token...");
        Write("\n");
        
        

        receptionists[ID].currentToken = ++TokenCounter;
        Release(TokenCounterLock);
        
            //Sleep till you get Acknowledgement

        print("R_");
        print(itoa(ID));
        print(":  Waiting for Patient to pick up token...");
        Write("\n");
        receptionists[ID].receptionistWaitCV->Wait(receptionists[ID].transLock);
        
            //Patient successfully got the token, go back to work: Loop again
        print("R_");
        print(itoa(ID));
        print(": Patient got token, Continue to next Patient");
        Write("\n");
        receptionists[ID].transLock->Release();

    }
    
}

void cashier(int ID) {

	
	  print("Cash_");
        print(itoa(ID));
        print(":  Alive!!");
	      Write("\n");
        while(true) {
        cashierLineLock->Acquire();
        
        if(cashiers[ID].lineLength > 0) { // someone in line
                                          //signal person on top
        print("Cash_");
        print(itoa(ID));
        print(":  someone in my line...");
        Write("\n");                                 
            
            cashiers[ID].lineCV->Signal(cashierLineLock);

        } else { // noone in line
                 // go on break
            
                // prefix for test condition
            if(test_state == 11)

        print("T11: ");
        print("Cash_");
        print(itoa(ID));
        print(":  No one in line... going on break");
        Write("\n");
            
            cashiers[ID].breakCV->Wait(cashierLineLock);
            cashierLineLock->Release();

            continue;
        }
        
            // I have a patient
            // acquire transLock and use it to govern transactions
            //  with the patient
        Acquire(cashiers[ID].transLock);
        Release(cashierLineLock);
        
            // waiting for patient to deposit its token in patToken
        Wait(cashiers[ID].transCV, cashiers[ID].transLock);
        
            // lookup value for cashiers[ID].patToken in the token table
        Acquire(feeListLock);
        cashiers[ID].fee = feeList->getValue(cashiers[ID].patToken);
        Release(feeListLock);
            // tell patient the fee
        
        Signal(cashiers[ID].transCV, cashiers[ID].transLock);
            // wait for payment
        Signal(cashiers[ID].transCV,cashiers[ID].transLock);
        
            // add this payment to our total collected
        Acquire(feesPaidLock);
        feesPaid += cashiers[ID].payment;
        cashiers[ID].sales += cashiers[ID].payment;
        Release(feesPaidLock);
        if(cashiers[ID].payment < cashiers[ID].fee) {
        	print("ERROR: call security, that patient didin't pay!");
            
        }        
        
        Release(cashiers[ID].transLock);
    }
}


void clerk(int ID){
    while(true){
        Acquire(ClerkLinesLock);
        
        if(clerks[ID].patientsInLine > 0) { // someone in line
                                            //signal the first person
            Signal(clerks[ID].ClerkCV, ClerkLinesLock);
        } else { // noone in line
                 // go on break
            
                // prefix for test condition
            if(test_state == 11)
            	print("T11: ");
                
            print("CL_");
            print(itoa(ID));
            print(": Going on break");
            Write("\n");
            
            
            Wait(clerks[ID].ClerkBreakCV, ClerkLinesLock);
            Release(ClerkLinesLock);
            continue;
        }
        
            // I have a patient
            // acquire the transaction Lock for further transactions
            //  with the patient
        Acquire(clerks[ID].ClerkTransLock);
        Release(ClerkLinesLock);
        
            // waiting for patient to give prescription
        Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
        
            // patient gives prescription:
        
            print("CL_");
            print(itoa(ID));
            print(": gave Medicines!");
            Write("\n");
        
        
        Signal(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
            // wait for payment
        Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
            //Collect payment
            
            print("CL_");
            print(itoa(ID));
            print(": The cost for the medicines are:");
            print(itoa(clerks[ID].fee));
            print(" Dollars");
            Write("\n");
        
        
            // add this payment to our total collected
        Acquire(PaymentLock);
        totalsales += clerks[ID].payment;
        clerks[ID].sales += clerks[ID].payment;
        Release(PaymentLock);
        
        
        Release(clerks[ID].ClerkTransLock);
    }
}


void hospitalManager(int ID){
	  print("H_");
	  print(itoa(ID));
	  print(": Alive");
	  Write("\n");
    
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
        Acquire(hospitalLock);
        if (peopleInHospital <= 0) {
        	print("H_");
	  print(itoa(ID));
	  print(":  No one to service, Killing myself!!!");
	  Write("\n");
           
            return;
        }
        Release(hospitalLock);
        
        sleeptime = Random() % 30000;
            //Sleep for some random amount of time
       print("H_");
	  print(itoa(ID));
	  print(":  Sleeping for");
	  print(itoa(sleeptime));
	  print(" cycles");
	  Write("\n");
            
       
        do{
            Yield();
            sleeptime--;
        }while (sleeptime > 0);
            //I am on rounds now, Time to kick some ass
        print("H_");
	      print(itoa(ID));
	      print(": Going on rounds");
        Write("\n");
        
        
        
            //1. Check on the Receptionists
        print("H_");
	      print(itoa(ID));
	      print(": Checking receptionists");
	      Write("\n");
        
        int patientsWaiting=0;
        for (int j=0; j<numRecp; j++) {
            patientsWaiting += receptionists[j].peopleInLine;
        }
        
        if (patientsWaiting > 1) {
            for (int j=0; j<numRecp; j++) {
                recpLineLock->Acquire();
                Signal(receptionists[j].ReceptionistBreakCV, recpLineLock);
                Release(recpLineLock);
            }
        }
            //2. Query Cashiers
            
        print("H_");
	      print(itoa(ID));
	      print(": Checking cashiers");
        Write("\n");
        for (int i=0; i<numCashiers; i++) {//Check for waiting patients
            if (cashiers[i].lineLength > 0 ) {
        
        print("H_");
	      print(itoa(ID));
	      print(": Found");
	      print(itoa(cashiers[i].lineLength));
	      print(" patients waiting for C_");
        print(itoa(i));
        print("  -> Signal Cashier");	      
        Write("\n");
                    //Wake up this receptionist up
                Acquire(cashierLineLock);
                Broadcast(cashiers[i].breakCV, cashierLineLock);
                Release(cashierLineLock);
                
            }
        }
        
            //Query cashiers for total sales

        feesPaidLock->Acquire();
        print(" T10: Total fees collected by cashiers:");
        print(itoa(feesPaid));
        

        if( test_state == 10 ) {
                // this is a test for race conditions, so we can't have any:
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            int sum = 0;
            for (int i=0; i<numCashiers; i++) {
            	print(" T10: cashier");
            	print(itoa(i));
            	print(" :");
            	print(itoa(cashiers[i].sales));
              Write("\n");  
              
                sum += cashiers[i].sales;
            }
            print("T10: TOTAL:");
            print(itoa(sum));
            
            
            
                // sum just printed should match feesPaid, printed earlier
            (void) interrupt->SetLevel(oldLevel);
        }
        Release(feesPaidLock);
        
        
            //3. Query pharmacy
        print("H_");
        print(itoa(ID));
        print(":Checking clerks");
        Write("\n");
        
        
        for (int i=0; i<numClerks; i++) {//Check for waiting patients
            if (clerks[i].patientsInLine > 0 ) {

            	 print("H_");
            	 print(itoa(ID));
               print(": found CL_");
               print(itoa(i));
               print(": sleeping and ");
               print(itoa(clerks[i].patientsInLine));
               print("waiting -> Signaling Clerk");
               Write("\n");
                //Wake up this clerk up
                ClerkLinesLock->Acquire();
                clerks[i].ClerkBreakCV->Signal(ClerkLinesLock);
                ClerkLinesLock->Release();

            }
        }
        
            //Query clerks for total sales

        PaymentLock->Acquire();
               print("H_");
            	 print(itoa(ID));
               print("T10: Total amount collected by clerks: ");
               print(itoa(totalsales));
               Write("\n");
        

        
        if( test_state == 10 ) {
                // this is a test for race conditions, so we can't have any:
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            int sum = 0;
            for (int i=0; i<numClerks; i++) {
            	
            	 print("T10: clerk ");
            	 print(itoa(i));
               print(" : ");
               print(itoa(clerks[i].sales));
               Write("\n");
               
                sum += clerks[i].sales;
            }
            print("T10: TOTAL: ");
            print(itoa(sum));
            
                // sum just printed should match feesPaid, printed earlier
            (void) interrupt->SetLevel(oldLevel);
        }
        Release(PaymentLock);
        
        Yield();
        
            //Check on the doorboys
               print("H_");
            	 print(itoa(ID));
            	 print(": Checking doorboys");
            	 Write("\n");
        
        for (int i=0; i<numDoctors; i++) {//Check for waiting patients
            if (doctors[i].peopleInLine > 0 ) {

            	  
            	 print("H_");
            	 print(itoa(ID));
               print(": found ");
               print(itoa(doctors[i].peopleInLine));
               print(": people in doctor ");
               print(itoa(i));
               print("'s line -> Signal Doorboy");
               Write("\n");
                
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
        
            //cout << "Simulation startup\n\n";
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
               print("Creating ");
            	 print(itoa(numCashiers));
               print("Cashiers");
               Write("\n");
        
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            print("Creating ");
            print(itoa(numDoorboys));
            print(" Doorboys");
            Write("\n");
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            print("Bypassing Doorboy Creation");
            Write("\n");
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            print("Creating ");
            print(itoa(numClerks));
            print(" Clerks");
            Write("\n");
        
        for(i=0;i<numClerks;i++)
        {
            
            Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            
            print("Creating ");
            print(itoa(numDoctors));
            print(" Doctors");
            Write("\n");
        for(i=0;i<numDoctors;i++)
        {
            
            Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        print("Creating ");
            print(itoa(numPatients));
            print(" Patients");
            Write("\n");
        
        for(i=0;i<numPatients;i++)
        {
            
            Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager

        print("Creating 1 Hospital Manager ");    
        Write("\n");
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   
   
        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
            print("Creating ");
            print(itoa(numRecp));
            print(" Receptionists");
            Write("\n");
        
        for(i=0; i<numRecp; i++)
        {
            Fork((VoidFunctionPtr) receptionist, i);
        }
        
        
    }else if (testmode == 51) {
        
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        
        
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            print("Creating ");
            print(itoa(numCashiers));
            print(" Cashiers");
            Write("\n");
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            print("Creating ");
            print(itoa(numDoorboys));
            print(" Doorboys");
            Write("\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            print(" Bypassing Doorboy Creation");
            Write("\n");
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
        
            print("Creating ");
            print(itoa(numClerks));
            print(" Clerks");
            Write("\n");
       
        for(i=0;i<numClerks;i++)
        {
            
            Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            print("Creating ");
            print(itoa(numDoctors));
            print(" Doctors");
            Write("\n");
        for(i=0;i<numDoctors;i++)
        {
            
            Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        print("Creating ");
            print(itoa(numPatients));
            print(" Patients");
            Write("\n");
        for(i=0;i<numPatients;i++)
        {
            
            Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager

            
        print("Creating 1 Hospital Manager "); 
        Write("\n");
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
            
            print("Creating ");
            print(itoa(numDoorboys));
            print(" Doorboys");
            Write("\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            print(" Bypassing Doorboy Creation");
            Write("\n");
        }
        
        
            //5. Pharmacys
        numClerks= (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            print("Creating ");
            print(itoa(numClerks));
            print(" Clerks");
            Write("\n");
        for(i=0;i<numClerks;i++)
        {
            
            Fork((VoidFunctionPtr) clerk, i);
        }
        
        
            //1. Doctors
            print("Creating ");
            print(itoa(numDoctors));
            print(" Doctors");
            Write("\n");
       
        for(i=0;i<numDoctors;i++)
        {
            
            Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
            print("Creating ");
            print(itoa(numPatients));
            print(" Patients");
            Write("\n");
        
        for(i=0;i<numPatients;i++)
        {
            
            t=new Thread(temp);
            t->Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager

            
            
        print("Creating 1 Hospital Manager ");     
        Write("\n");
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   

        
        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
            print("Creating ");
            print(itoa(numRecp));
            print(" Receptionists");
            Write("\n");
        
        for(i=0; i<numRecp; i++)
        {
            Fork((VoidFunctionPtr) receptionist, i);
        }
        
        
    }else if (testmode == 53) {
        int i = 0;
        char temp[] = "NACHOS_THREAD";
        
            //3. Cashiers
        numCashiers = (Random() % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            
            print("Creating ");
            print(itoa(numCashiers));
            print(" Cashiers");
            Write("\n");
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork((VoidFunctionPtr) cashier, i);
        }
        
            //4. DoorBoys
        numDoctors = (Random() % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == false){
            numDoorboys = numDoctors;
            
            print("Creating ");
            print(itoa(numDoorboys));
            print(" Doorboys");
            Write("\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork((VoidFunctionPtr) doorboy, i);
            }            
        }else{
            numDoorboys = 0;
            
            print(" Bypassing Doorboy Creation");
            Write("\n");
            
        }
        
        
            //5. No Pharmacy clerks
        numClerks = (Random() % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;        
        
            //1. Doctors
            print("Creating ");
            print(itoa(numDoctors));
            print(" Doctors");
            Write("\n");
           
       
        for(i=0;i<numDoctors;i++)
        {
            
            Fork((VoidFunctionPtr) doctor, i);
        }
        
        
            //7. Patients
        numPatients = Random() % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        
            print("Creating ");
            print(itoa(numPatients));
            print(" Patients");
            Write("\n");
        
        for(i=0;i<numPatients;i++)
        {
            
            Fork((VoidFunctionPtr) patients, i);
        }
        
        
        
            //6. HospitalManager

            
        print("Creating 1 Hospital Manager ");
        Write("\n");      
        t = new Thread("HospitalManager_0");
        t->Fork((VoidFunctionPtr) hospitalManager, 0);   

        
        
        
            //2. Receptionists
        numRecp = (Random() % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
        print("Creating ");
        print(itoa(numRecp));
        print(" Receptionists");
        Write("\n");
        
        for(i=0; i<numRecp; i++)
        {
            Fork((VoidFunctionPtr) receptionist, i);
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
