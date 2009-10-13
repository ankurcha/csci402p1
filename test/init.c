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
 



#include "init.h"
#include "patient.c"

int testmode=0;

void doorboy(ID){
    char str[50];
    int myDoctor = 0;
    char doorboyBreak = 0;
    
    while (1) {
        print("DB_");
        print(itoa(ID, str));
        print(": Alive ");
        
            /*Get into the doorboyLine till some doctor asks for me */
        Acquire(doorboyLineLock);
        
        doorboyLineLength++;

        print("DB_");
        print(itoa(ID, str));
        print(": Waiting for some doctor to wake me up.");
        print("\n");
        Wait(doorboyLineCV,doorboyLineLock);

        doorboyLineLength--;
        
            /*Some doctor woke me up, lets check who */
            /*myDoctor =  wakingDoctorID; */
        if(Queue_IsEmpty(&wakingDoctorList) == 1) {
        print("DB_");
        print(itoa(ID, str));
        print(": ERROR: Waking doctor list is empty!\n");
            continue;
        }
        myDoctor = Queue_Pop(&wakingDoctorList);
        if(test2active==1) {
            print("DB_");
            print(itoa(ID, str));
            print(":TEST2: Servicing D_");
            print(itoa(myDoctor,str));
            print("\n");
        } else {
            print("DB_");
            print(itoa(ID, str));
            print(":Servicing D_");
            print(itoa(myDoctor,str));
            print("\n");
        }
           
        Release(doorboyLineLock);

        
            /* Inform the doctor that I have arrived, and wait for him to take  */
            /*  a break, if he so chooses */
        Acquire(doctors[myDoctor].transLock);
        Signal(doctors[myDoctor].transCV,doctors[myDoctor].transLock);
        Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
        
            /*/// PATIENT LINE ///// */
            /*Acquire the lock to get the state of the line and take decision */

        Acquire(doctors[myDoctor].LineLock);
        print("DB_");
        print(itoa(ID, str));
        print(": Checking for Patients\n");
        
            /*while there is noone in line */
        doorboyBreak = 0;
        while(doctors[myDoctor].peopleInLine <= 0) { 
            doorboyBreak = 1;
                /*I will be woken up by the manager only!! */
            
                /* prefix for test conditions */
            if(myDoctor == 0 && test_state == 8)
            	print("T8: ");
                
            if(test_state == 11)
            	print("T11: ");
              if(test2active==1) {
            	print("DB_");
              print(itoa(ID, str));
               print(":TEST2: Yawn!!...ZZZZzzzzz....\n");
            } else {
            	print("DB_");
              print(itoa(ID, str));
               print(": Yawn!!...ZZZZzzzzz....\n");
                
            }
            Wait(doctors[myDoctor].doorboyBreakCV, doctors[myDoctor].LineLock);
                /* I got woken up, time to go back to work - by now there are  */
                /*  people dying on the floor! */
        }
        if(doorboyBreak) {
                /* prefix for test 8 condition */
            if(myDoctor == 0 && test_state == 8) {
            	print("T8: \n");
            
            
            }
            print("DB_");
            print(itoa(ID, str));
            print(": Woken up!\n");
        }
        
        print("DB_");
        print(itoa(ID, str));
        print(": Found ");
        print(itoa(doctors[myDoctor].peopleInLine,str));
       print(" patients waiting in line for D_ ");
        print(itoa(myDoctor,str));
        print("\n");
             
        
            /*Now wake the patient up to go to the doctor */

        print("DB_");
        print(itoa(ID, str));
        print(":Tell patient to go to doctor D_");
        print(itoa(myDoctor,str));
        print("\n");
        
        Signal(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);

        
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
            if(15 % 100 > 49) { /* go on break */
                doctorBreak = 1;
                    /* 5-15 yields */
                numYields = 5 + (15 % 11);
                
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
        numYields = 10 + (15 % 11);
        for(i=0; i < numYields; ++i) {
            Yield();  /* I see ... mm hmm ... does it hurt here? ... */
        }
        
            /* give prescription to patient */
        doctors[ID].prescription = 15 % 100;
        
            /* put consultation fees into the data structure for the cashier ($50-$250) */
        print("D_");
        print(itoa(ID, str));
        print(": Telling fee to cashiers\n");
        
        consultFee = (50 + (15 % 201));
        Acquire(feeListLock);
        List_Append(&feeList,doctors[ID].patientToken, consultFee);
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

void receptionist(int ID){
    char str[50];
    while (1) {

    	print("R_");
      print(itoa(ID, str));
      print(": Alive!\n");
        Acquire(recpLineLock);

        if (receptionists[ID].peopleInLine > 0) {
                /*Wake one waiting patient up */
            Signal(receptionists[ID].receptionCV, recpLineLock);
        } else {
                /*My Line is empty */
                /* prefix for test condition */
            if(test_state == 11)

            print( "T11: ");
            print("R_");
            print(itoa(ID, str));
            print(":Going to sleep\n");
            Wait(receptionists[ID].ReceptionistBreakCV, recpLineLock);
            Release(recpLineLock);

                /*HospitalManager kicked my ass for sleeping on the job!! */
                /*Loop back!! */
            continue;
        }
        
        Acquire(receptionists[ID].transLock);
        Release(recpLineLock);
        
            /*Genetate token for the patient */

        Acquire(TokenCounterLock);
        print("R_");
        itoa(ID, str);
        print(str);
        print(": Generating Token...\n");
        
        

        receptionists[ID].currentToken = ++TokenCounter;
        Release(TokenCounterLock);
        
            /*Sleep till you get Acknowledgement */

        print("R_");
        print(itoa(ID, str));
        print(":  Waiting for Patient to pick up token...\n");
        Wait(receptionists[ID].receptionistWaitCV, receptionists[ID].transLock);
        
            /*Patient successfully got the token, go back to work: Loop again */
        print("R_");
        print(itoa(ID, str));
        print(": Patient got token, Continue to next Patient\n");
        Release(receptionists[ID].transLock);

    }
    
    Yield();
    
}

void cashier(int ID) {

	char str[50];
	  print("Cash_");
        print(itoa(ID, str));
        print(":  Alive!!\n");
        while(1) {
        Acquire(cashierLineLock);
        
        if(cashiers[ID].lineLength > 0) { /* someone in line */
                                          /*signal person on top */
        print("Cash_");
        print(itoa(ID, str));
        print(":  someone in my line...\n");                                 
            
            Signal(cashiers[ID].lineCV, cashierLineLock);

        } else { /* noone in line */
                 /* go on break */
            
                /* prefix for test condition */
            if(test_state == 11)

        print("T11: ");
        print("Cash_");
        print(itoa(ID, str));
        print(":  No one in line... going on break\n");
            
            Wait(cashiers[ID].breakCV, cashierLineLock);
            Release(cashierLineLock);

            continue;
        }
        
            /* I have a patient */
            /* acquire transLock and use it to govern transactions */
            /*  with the patient */
        Acquire(cashiers[ID].transLock);
        Release(cashierLineLock);
        
            /* waiting for patient to deposit its token in patToken */
        Wait(cashiers[ID].transCV, cashiers[ID].transLock);
        
            /* lookup value for cashiers[ID].patToken in the token table */
        Acquire(feeListLock);
        cashiers[ID].fee = List_getValue(&feeList,cashiers[ID].patToken);
        Release(feeListLock);
            /* tell patient the fee */
        
        Signal(cashiers[ID].transCV, cashiers[ID].transLock);
            /* wait for payment */
        Signal(cashiers[ID].transCV,cashiers[ID].transLock);
        
            /* add this payment to our total collected */
        Acquire(feesPaidLock);
        feesPaid += cashiers[ID].payment;
        cashiers[ID].sales += cashiers[ID].payment;
        Release(feesPaidLock);
        if(cashiers[ID].payment < cashiers[ID].fee) {
        	print("ERROR: call security, that patient didin't pay!");
            
        }        
        
        Release(cashiers[ID].transLock);
    }
    Exit(0);
}


void clerk(int ID){
    char str[50];
    while(1){
        Acquire(ClerkLinesLock);
        
        if(clerks[ID].patientsInLine > 0) { /* someone in line */
                                            /*signal the first person */
            Signal(clerks[ID].ClerkCV, ClerkLinesLock);
        } else { /* noone in line */
                 /* go on break */
            
                /* prefix for test condition */
            if(test_state == 11)
            	print("T11: ");
                
            print("CL_");
            print(itoa(ID, str));
            print(": Going on break\n");
            
            
            Wait(clerks[ID].ClerkBreakCV, ClerkLinesLock);
            Release(ClerkLinesLock);
            continue;
        }
        
            /* I have a patient */
            /* acquire the transaction Lock for further transactions */
            /*  with the patient */
        Acquire(clerks[ID].ClerkTransLock);
        Release(ClerkLinesLock);
        
            /* waiting for patient to give prescription */
        Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
        
            /* patient gives prescription: */
        
            print("CL_");
            print(itoa(ID, str));
            print(": gave Medicines!\n");
        
        
        Signal(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
            /* wait for payment */
        Wait(clerks[ID].ClerkTransCV, clerks[ID].ClerkTransLock);
            /*Collect payment */
            
            print("CL_");
            print(itoa(ID, str));
            print(": The cost for the medicines are:");
            print(itoa(clerks[ID].fee,str));
            print(" Dollars\n");
        
        
            /* add this payment to our total collected */
        Acquire(PaymentLock);
        totalsales += clerks[ID].payment;
        clerks[ID].sales += clerks[ID].payment;
        Release(PaymentLock);
        
        
        Release(clerks[ID].ClerkTransLock);
    }
    Exit(0);
}


void hospitalManager(int ID) {
    char str[50];
    int sleeptime = 0;
    int test5cycles = 1;
    int patientsWaiting=0;
    int i, j, sum;

    print("H_");
    print(itoa(ID, str));
    print(": Alive\n");
    
    sleeptime = 15 % 30000;
    while (1) {
        if (test_state == 51 || test_state == 52 || test_state == 53) {
                /*The patients will always be there in the system. */
                /*For test purposes, lets assume the simulation to be */
                /*complete after 100 cycles. */
            if (test5cycles > 0) {
                test5cycles--;
            }else {
                return;
            }   
        }
        Acquire(hospitalLock);
        if (peopleInHospital <= 0) {
        	print("H_");
	  print(itoa(ID, str));
	  print(":  No one to service, Killing myself!!!\n");
           
            return;
        }
        Release(hospitalLock);
        
        sleeptime = 15 % 30000;
            /*Sleep for some random amount of time */
       print("H_");
	  print(itoa(ID, str));
	  print(":  Sleeping for");
	  print(itoa(sleeptime,str));
	  print(" cycles\n");
            
       
        do{
            Yield();
            sleeptime--;
        }while (sleeptime > 0);
            /*I am on rounds now, Time to kick some ass */
        print("H_");
	      print(itoa(ID, str));
	      print(": Going on rounds\n");
        
        
        
            /*1. Check on the Receptionists */
        print("H_");
	      print(itoa(ID, str));
	      print(": Checking receptionists\n");
        
        patientsWaiting=0;
        for (j=0; j<numRecp; j++) {
            patientsWaiting += receptionists[j].peopleInLine;
        }
        
        if (patientsWaiting > 1) {
            for (j=0; j<numRecp; j++) {
                Acquire(recpLineLock);
                Signal(receptionists[j].ReceptionistBreakCV, recpLineLock);
                Release(recpLineLock);
            }
        }
            /*2. Query Cashiers */
            
        print("H_");
	      print(itoa(ID, str));
	      print(": Checking cashiers\n");
        for (i=0; i<numCashiers; i++) {/*Check for waiting patients */
            if (cashiers[i].lineLength > 0 ) {
        
        print("H_");
	      print(itoa(ID, str));
	      print(": Found");
	      print(itoa(cashiers[i].lineLength, str));
	      print(" patients waiting for C_");
        print(itoa(i,str));
        print("  -> Signal Cashier\n");
                    /*Wake up this receptionist up */
                Acquire(cashierLineLock);
                Broadcast(cashiers[i].breakCV, cashierLineLock);
                Release(cashierLineLock);
                
            }
        }
        
            /*Query cashiers for total sales */

        Acquire(feesPaidLock);
        print(" T10: Total fees collected by cashiers:");
        print(itoa(feesPaid,str));
        

        if( test_state == 10 ) {
                /* this is a test for race conditions, so we can't have any: */
           /* IntStatus oldLevel = interrupt->SetLevel(IntOff);*/
            sum = 0;
            for (i=0; i<numCashiers; i++) {
            	print(" T10: cashier");
            	print(itoa(i,str));
            	print(" :");
            	print(itoa(cashiers[i].sales,str));
              print("\n");  
              
                sum += cashiers[i].sales;
            }
            print("T10: TOTAL:");
            print(itoa(sum,str));
            
            
            
                /* sum just printed should match feesPaid, printed earlier */
          /*  (void) interrupt->SetLevel(oldLevel);*/
        }
        Release(feesPaidLock);
        
        
            /*3. Query pharmacy */
        print("H_");
        print(itoa(ID, str));
        print(":Checking clerks\n");
        
        
        for (i=0; i<numClerks; i++) {/*Check for waiting patients */
            if (clerks[i].patientsInLine > 0 ) {

            	 print("H_");
            	 print(itoa(ID, str));
               print(": found CL_");
               print(itoa(i,str));
               print(": sleeping and ");
               print(itoa(clerks[i].patientsInLine,str));
               print("waiting -> Signaling Clerk\n");
                /*Wake up this clerk up */
                Acquire(ClerkLinesLock);
                Signal(clerks[i].ClerkBreakCV, ClerkLinesLock);
                Release(ClerkLinesLock);

            }
        }
        
            /*Query clerks for total sales */

        Acquire(PaymentLock);
               print("H_");
            	 print(itoa(ID, str));
               print("T10: Total amount collected by clerks: ");
               print(itoa(totalsales,str));
               print("\n");
        

        
        if( test_state == 10 ) {
                /* this is a test for race conditions, so we can't have any: */
         /*   IntStatus oldLevel = interrupt->SetLevel(IntOff);*/
            sum = 0;
            for (i=0; i<numClerks; i++) {
            	
            	 print("T10: clerk ");
            	 print(itoa(i,str));
               print(" : ");
               print(itoa(clerks[i].sales,str));
               print("\n");
               
                sum += clerks[i].sales;
            }
            print("T10: TOTAL: ");
            print(itoa(sum,str));
            
                /* sum just printed should match feesPaid, printed earlier */
           /* (void) interrupt->SetLevel(oldLevel);*/
        }
        Release(PaymentLock);
        
        Yield();
        
            /*Check on the doorboys */
               print("H_");
            	 print(itoa(ID, str));
            	 print(": Checking doorboys\n");
        
        for (i=0; i<numDoctors; i++) {/*Check for waiting patients */
            if (doctors[i].peopleInLine > 0 ) {

            	  
            	 print("H_");
            	 print(itoa(ID, str));
               print(": found ");
               print(itoa(doctors[i].peopleInLine,str));
               print(": people in doctor ");
               print(itoa(i,str));
               print("'s line -> Signal Doorboy\n");
                
                Acquire(doctors[i].LineLock);
                Broadcast(doctors[i].doorboyBreakCV, doctors[i].LineLock);
                Release(doctors[i].LineLock);

            }
        }        
    }
    Exit(0);
}

void createPatient(){
    int temp;
    Acquire(creationLock);
    temp = patientCount;
    patientCount++;
    Release(creationLock);
    patients(temp);
    Exit(0);
}

void createReceptionist(){
    int temp;
    Acquire(creationLock);
    temp = recptionistCount;
    recptionistCount++;
    Release(creationLock);
    receptionist(temp);
    Exit(0);
}

void createDoorBoy(){
    int temp;
    Acquire(creationLock);
    temp = doorboyCount;
    doorboyCount++;
    Release(creationLock);
    doorboy(temp);
    Exit(0);
}

void createDoctor(){
    int temp;
    Acquire(creationLock);
    temp = doctorCount;
    doctorCount++;
    Release(creationLock);
    doctor(temp);
    Exit(0);
    
}

void createCashier(){
    int temp;
    Acquire(creationLock);
    temp = cashierCount;
    cashierCount++;
    Release(creationLock);
    cashier(temp);
    Exit(0);
}

void createPharmacyClerk(){
    int temp;
    Acquire(creationLock);
    temp = pharmacyCount;
    pharmacyCount++;
    Release(creationLock);
    clerk(temp);
    Exit(0);
}

void createHospitalManager(){
    int temp;
    Acquire(creationLock);
    temp = hospitalmanagerCount;
    hospitalmanagerCount++;
    Release(creationLock);
    hospitalManager(temp);
    Exit(0);
}


void HospINIT(int testmode) {
    char str[50];
    int i;
    
        /* set a global so everyone will know the test mode */
    test_state = testmode;
    
    if(testmode != 1 && testmode != 51 && testmode != 52 && testmode != 53 ){
        i = 0;
        
            /*cout << "Simulation startup\n\n"; */
        
            /*3. Cashiers */
        numCashiers = (15 % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
               print("Creating ");
            	 print(itoa(numCashiers,str));
               print("Cashiers\n");
        
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork(createCashier);
        }
        
            /*4. DoorBoys */
        numDoctors = (15 % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == 0){
            numDoorboys = numDoctors;
            print("Creating ");
            print(itoa(numDoorboys,str));
            print(" Doorboys\n");
            for(i=0;i<numDoorboys;i++)
            {
                Fork(createDoorBoy);
            }            
        }else{
            numDoorboys = 0;
            print("Bypassing Doorboy Creation\n");
        }
        /*5. Pharmacys */
        numClerks= (15 % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            print("Creating ");
            print(itoa(numClerks,str));
            print(" Clerks\n");
        
        for(i=0;i<numClerks;i++)
        {
            Fork(createPharmacyClerk);
        }
        /*1. Doctors */
        print("Creating ");
        print(itoa(numDoctors,str));
        print(" Doctors\n");
        for(i=0;i<numDoctors;i++)
        {
            
            Fork(createDoctor);
        }
        
        
            /*7. Patients */
        numPatients = 15 % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        print("Creating ");
            print(itoa(numPatients,str));
            print(" Patients\n");
        
        for(i=0;i<numPatients;i++)
        {
            
            Fork(createPatient);
        }
        
        
        
            /*6. HospitalManager */

        print("Creating 1 Hospital Manager \n");
        
        
        Fork(createHospitalManager);   
   
        
        
        
            /*2. Receptionists */
        numRecp = (15 % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
            print("Creating ");
            print(itoa(numRecp,str));
            print(" Receptionists\n");
        
        for(i=0; i<numRecp; i++)
        {
            Fork(createReceptionist);
        }
        
        
    }
    
    
    else if (testmode == 51) {
        
        i = 0;
        
        
        
            /*3. Cashiers */
        numCashiers = (15 % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            print("Creating ");
            print(itoa(numCashiers,str));
            print(" Cashiers\n");
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork(createCashier);
        }
        
            /*4. DoorBoys */
        numDoctors = (15 % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == 0){
            numDoorboys = numDoctors;
            print("Creating ");
            print(itoa(numDoorboys,str));
            print(" Doorboys\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork(createDoorBoy);
            }            
        }else{
            numDoorboys = 0;
            print(" Bypassing Doorboy Creation\n");
        }
        
        
            /*5. Pharmacys */
        numClerks= (15 % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
        
            print("Creating ");
            print(itoa(numClerks,str));
            print(" Clerks\n");
       
        for(i=0;i<numClerks;i++)
        {
            
            Fork(createPharmacyClerk);
        }
        
        
            /*1. Doctors */
            print("Creating ");
            print(itoa(numDoctors,str));
            print(" Doctors\n");
        for(i=0;i<numDoctors;i++)
        {
            
            Fork(createDoctor);
        }
        
        
            /*7. Patients */
        numPatients = 15 % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        print("Creating ");
            print(itoa(numPatients,str));
            print(" Patients\n");
        for(i=0;i<numPatients;i++)
        {
            
            Fork(createPatient);
        }
        /*6. HospitalManager */
        print("Creating 1 Hospital Manager \n");

       
        
        Fork(createHospitalManager);   

        
        
        
        
            /*2. No Receptionists */
       

        numRecp = (15 % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
    }else if (testmode == 52) {
        i = 0;
              
                    /*3. No Cashiers */
        numCashiers = (15 % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
        
        
            /*4. DoorBoys */
        numDoctors = (15 % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == 0){
            numDoorboys = numDoctors;
            
            print("Creating ");
            print(itoa(numDoorboys,str));
            print(" Doorboys\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork(createDoorBoy);
            }            
        }else{
            numDoorboys = 0;
            print(" Bypassing Doorboy Creation\n");
        }
        
        
            /*5. Pharmacys */
        numClerks= (15 % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;
            print("Creating ");
            print(itoa(numClerks,str));
            print(" Clerks\n");
        for(i=0;i<numClerks;i++)
        {
            
            Fork(createPharmacyClerk);
        }
        
        
        /*1. Doctors */
        print("Creating ");
        print(itoa(numDoctors,str));
        print(" Doctors\n");
        for(i=0;i<numDoctors;i++)
        {   
            Fork(createDoctor);
        }
        /*7. Patients */
        numPatients = 15 % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
            print("Creating ");
            print(itoa(numPatients,str));
            print(" Patients\n");
        
        for(i=0;i<numPatients;i++)
        {
            Fork(createPatient);
        }
        /*6. HospitalManager */
        print("Creating 1 Hospital Manager \n");

        
        Fork(createHospitalManager);   

        
        
        
        
            /*2. Receptionists */

        numRecp = (15 % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        print("Creating ");
        print(itoa(numRecp,str));
        print(" Receptionists\n");
        for(i=0; i<numRecp; i++)
        {
            Fork(createReceptionist);
        }
    }else if (testmode == 53) {
        i = 0;
        /*3. Cashiers */
        numCashiers = (15 % (MAX_CASHIER - MIN_CASHIER +1) + MIN_CASHIER) ;
            
            print("Creating ");
            print(itoa(numCashiers,str));
            print(" Cashiers\n");
        
        for(i=0;i<numCashiers;i++)
        {
            
            Fork(createCashier);
        }
        
            /*4. DoorBoys */
        numDoctors = (15 % (MAX_DOCTORS - MIN_DOCTORS + 1) + MIN_DOCTORS);
        if(test1active == 0){
            numDoorboys = numDoctors;
            
            print("Creating ");
            print(itoa(numDoorboys,str));
            print(" Doorboys\n");
            
            for(i=0;i<numDoorboys;i++)
            {
                
                Fork(createDoorBoy);
            }            
        }else{
            numDoorboys = 0;
            
            print(" Bypassing Doorboy Creation\n");
            
        }
        
        
            /*5. No Pharmacy clerks */
        numClerks = (15 % (MAX_CLERKS - MIN_CLERKS +1) + MIN_CLERKS) ;        
        
            /*1. Doctors */
            print("Creating ");
            print(itoa(numDoctors,str));
            print(" Doctors\n");
           
       
        for(i=0;i<numDoctors;i++)
        {
            
            Fork(createDoctor);
        }
        
        
            /*7. Patients */
        numPatients = 15 % (MAX_PATIENTS - MIN_PATIENTS +1) + MIN_PATIENTS; 
        Acquire(hospitalLock);
        peopleInHospital = numPatients;
        Release(hospitalLock);    
        
        
            print("Creating ");
            print(itoa(numPatients,str));
            print(" Patients\n");
        
        for(i=0;i<numPatients;i++)
        {
            
            Fork(createPatient);
        }
        
        
        
            /*6. HospitalManager */

            
        print("Creating 1 Hospital Manager \n");      

        Fork(createHospitalManager);   
        /*2. Receptionists */

        numRecp = (15 % (RECP_MAX - RECP_MIN +1) + RECP_MIN) ;
        
        print("Creating ");
        print(itoa(numRecp,str));
        print(" Receptionists\n");
        
        for(i=0; i<numRecp; i++)
        {
            Fork(createReceptionist);
        }
    }else if (testmode == 2) {
    }
}


int test1(){
    test1active = 1;
    HospINIT(0);
    return 0;
}

int test2(){
	test2active=1;
	HospINIT(0);
	return 0;
}

int test4(){
    test4active = 1;
        /*start the process normally */
    HospINIT(0);
    return 0;
    
}


int test7(){
    test7active = 1;
    HospINIT(0);
    return 0;
}


int main(int argc, char** argv){
    int i;
    char inp[20];
    print("Hello World\n");    
    testlock = CreateLock("TestLock");
    TokenCounterLock = CreateLock("TokenCounterLock");
    recpLineLock = CreateLock("recpLineLock");
    feeListLock = CreateLock("feeListLock");
    cashierLineLock = CreateLock("cashierLineLock");
    feesPaidLock = CreateLock("feesPaidLock");
    ClerkLinesLock= CreateLock("ClerkLineLock");
    PaymentLock= CreateLock("PaymentLock");
    hospitalLock = CreateLock("HospitalLock");
    doorboyLineLock = CreateLock("doorboyLineLock");
    doorboyLineCV = CreateCondition("doorboyLineCV");
    wakingDoctorList.queue = wakingdoctor_element;
    wakingDoctorList.length =  MAX_PATIENTS;
    wakingDoctorList.head = -1;
    wakingDoctorList.tail = -1; 
    Init_Queue(&wakingDoctorList);
    creationLock = CreateLock("creationLock");
        /*Initialize datastructures for all the threads
        //1. Patients don't need initialization
        //2. Receptionists
         */
    Write("Initializing Recptionists\n",25,1);
    for (i=0; i<RECP_MAX; i++) {
        __Receptionists(&receptionists[i]);
    }
        /*3. DoorBoy doesn't need anything
        4. Doctors*/
    print("Initializing Doctors\n");
    for (i=0; i<MAX_DOCTORS; i++) {
        __Doctor(&doctors[i]);
    }
    print("Initializing Cashiers\n");
        /*5. Cashiers*/
    for (i=0; i<MAX_CASHIER; i++) {
        __Cashier(&cashiers[i]);
    }
    print("Initializing Clerks\n");
        /*6. Clerks */
    for (i=0; i<MAX_CLERKS; i++) {
        __PharmacyClerks(&clerks[i]);
    }
        /* 7. Hospital Manager */
    print("Initializing Hospital Manager\n");
    for (i=0; i<totalHospMan; i++) {
    }
    
    Read(inp, 20, ConsoleInput);
    print(inp);
    switch (*(inp) ) {
        case '0':
            HospINIT(0);
            
            break;
        case '1':
            test1();
            
            break;
        case '2':
            test2();
            
            break;    
        case '3':
            HospINIT(3);
            
            break;
        case '4':
            test4();
            
            break;
        case '5':
            if(*(*(argv+1)+1) == '1'){
                HospINIT(51);
                break;    
            } if(*(*(argv+1)+1) == '2'){ 
                HospINIT(52);
            
                break;
            }if(*(*(argv+1)+1) == '3'){
                HospINIT(53);
                break;
            }
            break;
        case '6':
            HospINIT(6);
            
            break;
        case '7':
            test7();
            
            break;
        case '8':
            HospINIT(8);
            
            break;
        case '9':
            HospINIT(9);
            
            break;
        case 10:
            if(*(*(argv+1)+1) == '0'){
                HospINIT(10);
                break;    
            } if(*(*(argv+1)+1) == '1'){ 
                HospINIT(11);
            
                break;
            }
            break;
        default:
            HospINIT(0);
            break;
    }
    
       /*HospINIT(testmode);*/
    
    for(i=0;i<100;i++)
        Yield();
}
