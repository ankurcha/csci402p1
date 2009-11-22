#include "init.h"
int testmode=0;

void doctor(ID){
    char str[50];
    /* declare variables */
    int waitingtime = 10000;
    int i, numYields, consultFee;
    char doctorBreak = 0;
    
    
    while(1) {
        /* acquire a doorboy */
        
        print("D_");
        print(itoa(ID, str));
        print(": Alive!!\n");
        
        Acquire(doorboyLineLock);
        
        
        /* assure that there is a doorboy in line */
        while(doorboyLineLength <= 0) {
            if(waitingtime % 100 == 0){
                
            	print("D_");
                print(itoa(ID, str));
                print(": Doctor could not find a doorboy waittime: ");
                print(itoa(waitingtime,str));
                print("\n");
                
            }
            
            Release(doorboyLineLock);
            Yield();
            waitingtime--;
            Acquire(doorboyLineLock);
            if(waitingtime <= 0){
                print("Waited for a long time with no Doorboys, exiting...\n");
                return;
            }
        }
        
        /* pull the next doorboy off the line */
        
        print("D_");
        print(itoa(ID, str));
        print(":Signaling doorboy!\n");
        
        
        /*wakingDoctorID = ID; */
        Queue_Push(&wakingDoctorList, ID);
        Signal(doorboyLineCV,doorboyLineLock);
        
        /* acquire the transaction lock and wait for the doorboy to arrive */
        Acquire(doctors[ID].transLock);
        Release(doorboyLineLock);
        
        /*////  DOORBOY INTERACTION  ////// */
        Wait(doctors[ID].transCV, doctors[ID].transLock);
        
        doctorBreak = 0;
        /* go on break if so inclined */
        
        if(test7active==1)
       	{
            numYields = 35;
            
       		print("D_");
            print(itoa(ID, str));
            print(" :TEST7: Going on break for ");
            print(itoa(numYields,str));
            print(" cycles!\n");
            
            for(i=0; i < numYields; ++i) {
                Yield();
            }
       		
       	}
        else
            if(Random() % 100 > 49) { /* go on break */
                doctorBreak = 1;
                /* 5-15 yields */
                numYields = 5 + (Random() % 11);
                
                
                /* provide a handle for test 8, only uses doctor 0 */
                if(ID == 0 && test_state == 8 ) { 
                	print("T8: ");
                    
                }
                
                print("D_");
                print(itoa(ID, str));
                print(": Going on break for ");
                print(itoa(numYields,str));
                print(" cycles!\n");
                
                for(i=0; i < numYields; ++i) {
                    Yield();
                }
            }
        
        
        
        /* provide a handle for test 8, only uses doctor 0 */
        if(ID == 0 && test_state == 8 && doctorBreak) { 
        	print("T8: ");
            
        }
        if(doctorBreak) 
        	print("D_");
        print(itoa(ID, str));
        
        print(": Back from Break\n");
        
        /* inform the doorboy that I am ready for a patient */
        
        if(test7active==1)
       	{
       		print("D_");
        	print(itoa(ID, str));
       		print(":TEST7: Back from Break,Signalling patient to come in.\n");
       		
       	}
       	else
       		print("D_");
        print(itoa(ID, str));
        print(": Back from Break,Signalling patient to come in.\n");
        
        
        
        
        Signal(doctors[ID].transCV, doctors[ID].transLock);
        
        print("D_");
        print(itoa(ID, str));
        print(": Waiting for patient....\n");
        
        
        /*////  PATIENT INTERACTION  ////// */
        /* and wait for that patient to arrive */
        Wait(doctors[ID].transCV, doctors[ID].transLock);
        
        /* consult: 10-20 yields */
        
        print("D_");
        print(itoa(ID, str));
        print(": Now Consulting patient\n");
        
        numYields = 10 + (Random() % 11);
        for(i=0; i < numYields; ++i) {
            Yield();  /* I see ... mm hmm ... does it hurt here? ... */
        }
        
        /* give prescription to patient */
        doctors[ID].prescription = Random() % 100;
        
        /* put consultation fees into the data structure for the cashier ($50-$250) */
        
        print("D_");
        print(itoa(ID, str));
        print(": Telling fee to cashiers\n");
        
        consultFee = (50 + (Random() % 201));
        Acquire(feeListLock);
        List_Append(&feeList, doctors[ID].patientToken, consultFee);
        Release(feeListLock);
        
        /* pass the prescription to the patient and wait for them to leave */
        
        print("D_");
        print(itoa(ID, str));
        print(": Waiting for the patient to leave\n");
        
        Signal(doctors[ID].transCV, doctors[ID].transLock);
        Wait(doctors[ID].transCV, doctors[ID].transLock);
        
        /* done, the patient has left */
        Release(doctors[ID].transLock);
        
        print("D_");
        print(itoa(ID, str));
        print(": I'm ready for another one\n");
        
        
    } /*end while */
    Yield();
}

int main(int argc, char** argv){
    int i;
    char inp[20];
    strcpy(testlock,"TestLock");
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
    wakingDoctorList.length =  MAX_PATIENTS;
    wakingDoctorList.head = -1;
    wakingDoctorList.tail = -1; 
    Init_Queue(&wakingDoctorList);
    feeList.head = 0;
    /*Initialize datastructures for all the threads
     //1. Patients don't need initialization
     //2. Receptionists
     */
    Write("Initializing Recptionists DS\n",25,1);
    for (i=0; i<RECP_MAX; i++) {
        __Receptionists(&receptionists[i], i);
    }
    /*3. DoorBoy doesn't need anything
     4. Doctors*/
    print("Initializing Doctors DS\n");
    for (i=0; i<MAX_DOCTORS; i++) {
        __Doctor(&doctors[i], i);
    }
    print("Initializing Cashiers DS\n");
    /*5. Cashiers*/
    for (i=0; i<MAX_CASHIER; i++) {
        __Cashier(&cashiers[i], i);
    }
    print("Initializing Clerks DS\n");
    /*6. Clerks */
    for (i=0; i<MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i], i);
    }
    /* 7. Hospital Manager */
    print("Initializing Hospital Manager DS\n");
    for (i=0; i<totalHospMan; i++) {
        
    }
    
    /*1. Doctors */
    
    print("Creating "); 
    print(itoa(numDoctors,str)); 
    print(" Doctors\n");
    for(i=0;i<numDoctors;i++)
        Fork(createDoctor);
    
    for(i=0;i<100;i++)
        Yield();
}

