/*    Hospital Management Simulation */
/* */
/*    Patient Thread Function */
/*  */
/* CS 402 Fall 2009 */
/* Group 11 */
/*  Ankur Chauhan, ankurcha */
/*  Max Pflueger, pflueger */
/*  Aneesha Mathew, aneesham */

char test_code2=0;
void patients(int ID){
    
    /* declare variables */
    int myToken;
    int myDoctor;
    int myPrescription;

    int shortestline = 0;
    int len = 0;
    int i = 0;
    int myCashier = 0;
    int sLen = 0;

    len = receptionists[0].peopleInLine;

    /*//////////////////////////////////////////////// */
    /*//// Begin interaction with Receptionist /////// */
    /*//////////////////////////////////////////////// */

    print("P:Attempt to acquire recpLineLock...\n");
    Acquire(recpLineLock);
    print("P:success\n");

    /*Find shortest Line */
    if (test4active == 1) {
        print("P:TEST4: Searching for the receptionist with the shortest line\n");
    }else {
        print("P: Searching for the receptionist with the shortest line\n");
        
    }
    
    for (i=0; i<numRecp; i++) {
        
        if (test4active == 1) {
                /*Print the length of each line */
            print("P:TEST4: Length of line at R_");
            print(itoa(i));
            print(" = ");
            print(itoa(receptionists[i].peopleInLine));
            print("\n");
        }
        
        if(receptionists[i].peopleInLine < len){
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    
    if (test4active == 1) {
        print("P_");
        print(itoa(ID));
        print(":TEST4: Found shortest line with R_");
        print(itoa(shortestline));print(" len: ");print(itoa(len));
        print("\n");
    }else{
        print("P_");print(itoa(ID));print(": Found shortest line with R_");
        print(itoa(shortestline));print(" len: ");print(itoa(len));print("\n");
    }

    /* Wait in line */
    receptionists[shortestline].peopleInLine++;
   Wait(receptionists[shortestline].receptionCV,(recpLineLock));
    print("P_");
    print(itoa(ID));
    print(" Got woken up, get out of line and going to counter for token\n");
    receptionists[shortestline].peopleInLine--;
    
    /*wait for the receptionist to prepare token for me, till then I wait */
    Release(recpLineLock);
    Acquire(receptionists[shortestline].transLock);

    /*token is ready just read it -- print it out in our case */
    print("P_");
    print(itoa(ID));
    print(": Reading Token..\n");
    myToken = receptionists[shortestline].currentToken;
    print("P_");
    print(itoa(ID));
    print(": My token is ");
    print(itoa(ID));
    print("..yeah!!\n");
    /*Done, signal receptionist that he can proceed  */
    Signal(receptionists[shortestline].receptionistWaitCV, receptionists[shortestline].transLock);
    print("P_");
    print(itoa(ID));
    print(": Signal receptionist R_");
    print(itoa(shortestline));
    print(" to continue, I am done\n");
    /*Release transaction lock */
    Release(receptionists[shortestline].transLock);
    
    /*///////////////////////////////////////////////// */
    /*///// Interaction with Doctor and Doorboy /////// */
    /*///////////////////////////////////////////////// */

    /*Calculate which doctor I want to see */

    myDoctor = (int)(Random()) % numDoctors;
    if(test2active==1)
	{
	    print("P_");
            print(itoa(ID));
            print(" :TEST2: Going to meet doctor D_");
            print(itoa(myDoctor));
            print("\n");
	}
	 if(test7active==1)
	{
	    print("P_");
            print(itoa(ID));
            print(" :TEST7: Waiting in doctor D_");
            print(itoa(myDoctor));
           print(" Queue\n");
	}else{
            print("P_");
            print(itoa(ID));
            print(" : Going to meet doctor D_\n");
        }

    /* Acquire doc's line lock */
    Acquire(doctors[myDoctor].LineLock);

    /* Wait on the line -- to be woken up by the doorboy */
    if(test2active==1)
	{
            print("P_");
            print(itoa(ID));
	    print(" :TEST2: Join line and Waiting for doorboy to tell me to go\n");
	}
	else{
            print("P_");
            print(itoa(ID));
	    print(" : Join line and Waiting for doorboy to tell me to go\n");
        }
    doctors[myDoctor].peopleInLine++;
    Wait(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);
    print("P_");
    print(itoa(ID));
    print(" : Doorboy told me to go to doctor, proceeding....\n");
    doctors[myDoctor].peopleInLine--;

    /* move into the doctor's transaction lock */
    Release(doctors[myDoctor].LineLock);
      
    
    print("P_");
    print(itoa(ID));
    print(" : Trying to acquire doctor's translock\n");
   
    Acquire(doctors[myDoctor].transLock);
    print("P_");
    print(itoa(ID));
    print(" : Success\n");

    print("P_");
    print(itoa(ID));
    print(" : Giving doctor the token...\n");
    /*The doctor is waiting for me to provide my info, oblige him!! */
    doctors[myDoctor].patientToken = myToken;

    /* hand off to the doctor thread for consultation */
     if(test7active==1)
	{
	    print("P_");
            print(itoa(ID));
            print(" :TEST7: Consulting Doctor D_");
            print(itoa(myDoctor));
            print(" now...\n");
	}
	else{
            print("P_");
            print(itoa(ID));
            print(" : Consulting Doctor D_");
            print(itoa(myDoctor));
            print(" now...\n");
        }
    Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);

    /* Consultation finished, now I have to get the prescription from the doctor */
    myPrescription = doctors[myDoctor].prescription;
    print("P_");
    print(itoa(ID));
    print(" : Consultation finished and  Got prescription ");
    print(itoa(myPrescription));
    print("\n");

    /* Signal the doctor that I have taken the prescription and left */
    Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    Release(doctors[myDoctor].transLock);
    print("P_");
    print(itoa(ID));
    print(": Done with Doctor, going to cashier.\n");
    /*////////////////////////////////////////// */
    /*///////  Interaction with Cashier //////// */
    /*////////////////////////////////////////// */
    
    Acquire(cashierLineLock);
    print("P_");
    print(itoa(ID));
    print(": Acquiring cashierLineLock\n");

    /* find the shortest line */
    sLen = cashiers[0].lineLength;
    if (test4active == 1) {
        print("P_");
        print(itoa(ID));
        print(":TEST4: Finding shortest Line of cashiers\n");
    }else {
        print("P_");
        print(itoa(ID));
        print(": Finding shortest Line of cashiers\n");
    }

    for(int i=1; i < numCashiers; ++i) {
        if (test4active == 1) {
                /*Print the length of each line */
            print("P_");
            print(itoa(ID));
            print(":TEST4: Length of line at R_");
            print(itoa(i));
            print(" = ");
            print(itoa(cashiers[i].lineLength));
            print("\n");
        }
        
        if(cashiers[i].lineLength < sLen) {
            myCashier = i;
            sLen = cashiers[i].lineLength;
        }
    }
    
    if (test4active == 1) {
        print("P_");
        print(itoa(ID));
        print(":TEST4: Found shortest cashier line C_");
        print(itoa(myCashier));
        print("len: ");
        print(itoa(sLen));
        print("\n");
    }else {
    	     print("P_");
        print(itoa(ID));
        print(": Found shortest cashier line C_");
        print(itoa(myCashier));
        print("len: ");
        print(itoa(sLen));
        print("\n");
    	
        
    }

    /*if(sLen > 0) {get in line} else {get in line} */
    /* there are a lot of cases here, but they all result in us getting in line */
    cashiers[myCashier].lineLength ++;
    print("P_");
    print(itoa(ID));
    print(": Waiting in line for cashier C_");
    print(itoa(myCashier));
    print(" to attend to me, Line length: ");
    print(itoa(cashiers[myCashier].lineLength));
    print("\n");
    Wait(cashiers[myCashier].lineCV,cashierLineLock);
    print("P_");
    print(itoa(ID));
    print(": Going to meet cashier C_");
    print(itoa(myCashier));
    print("\n");
    cashiers[myCashier].lineLength --;

    /*// APPROACH THE DESK //// */
    Release(cashierLineLock);
    Acquire(cashiers[myCashier].transLock);

    /* provide token to cashier */
    cashiers[myCashier].patToken = myToken;

    /* wait for cashier to come back with the fee */
    Signal(cashiers[myCashier].transCV, cashiers[myCashier].transLock);
    Wait(cashiers[myCashier].transCV, cashiers[myCashier].transLock);

    /* provide the money */
    cashiers[myCashier].payment = cashiers[myCashier].fee;
    print( "P_");
    print(itoa(ID));
    print(": Paying money.\n");
    /* done */
    Signal(cashiers[myCashier].transCV, cashiers[myCashier].transLock);
    Release(cashiers[myCashier].transLock);
    print("P_");
    print(itoa(ID));
    print(": Done with cashier\n");
    /*//////////////////////////////////////////////// */
    /*//////  Interaction with Pharmacy Clerk //////// */
    /*//////////////////////////////////////////////// */
    
    print("P_");
    print(itoa(ID));
    print(":Attempt to acquire ClerkLinesLock...\n");
    Acquire(ClerkLinesLock)
    print("success\n");
    int shortestclerkline = 0;
    int length = clerks[0].patientsInLine;
    if (test4active == 1) {
        print("P_");
        print(itoa(ID));
        print(":TEST4: Finding shortest Line of clerks\n");
    }else {
        print("P_");
        print(itoa(ID));
        print(": Finding shortest Line of clerks\n");
    }
    
    /*Find shortest Line */
    for (int i=0; i<numClerks; i++) {
        if (test4active == 1) {
                /*Print the length of each line */
            print("P_");
            print(itoa(ID));
            print(": TEST4: Length of line at CL_");
            print(itoa(i));
            print(" = ");
            print(itoa(clerks[i].patientsInLine));
            print("\n");
        }
        if(clerks[i].patientsInLine < length){
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }

    
    if (test4active == 0) {
        print("P_");
        print(itoa(ID));
        print(": Found shortest pharmacy clerk line CL_");
        print(itoa(shortestclerkline));
        print(" len: ");
        print(itoa(length));
        print("\n");
    }else {
        print("P_");
        print(itoa(ID));
        print(":TEST4: Found shortest pharmacy clerk line CL_");
        print(itoa(shortestclerkline));
        print(" len: ");
        print(itoa(length));
        print("\n");
    }

        /*wait in line for my turn */
    clerks[shortestclerkline].patientsInLine++;
    
    print("P_");
        print(itoa(ID));
        print(": Waiting in line for clerk CL_");
        print(itoa(shortestclerkline));
        print(" to attend to me, Line length:: ");
        print(itoa(clerks[shortestclerkline].patientsInLine));
        print("\n");
    
      
    Wait(clerks[shortestclerkline].ClerkCV, ClerkLinesLock);
    
    print("P_");
    print(itoa(ID));
    print(" Got woken up, got out of line and going to the Pharmacy CLerk to give prescription.\n");
    clerks[shortestclerkline].patientsInLine--;
    
    Release(ClerkLinesLock);
    Acquire(clerks[shortestclerkline].ClerkTransLock);
        /*signal ParmacyClerk that i am ready to give Prescription */
    print("P_");
    print(itoa(ID));
    print(": Acquired ClerkTransLock\n");
        /*Entered the line no need to hold all lines others may now continue */
    /*wait for the PharmacyClerk to Get the prescription from me.. so I wait */
     clerks[shortestclerkline].patPrescription = myPrescription;
    print( "P_");
    print(itoa(ID));
    print(": Gave prescriptiong, waiting for medicines.\n");
    /* wait for clerk to give cost */
    Signal(clerks[shortestclerkline].ClerkTransCV, clerks[shortestclerkline].ClerkTransLock);
    Wait(clerks[shortestclerkline].ClerkTransCV, clerks[shortestclerkline].ClerkTransLock);

    /* provide the money */
    print( "P_");
    print(itoa(ID));
    print(": Got Medicines, making payment.\n");
    clerks[shortestclerkline].payment = clerks[shortestclerkline].fee;

    /* done */
    Signal(clerks[shortestclerkline].ClerkTransCV,clerks[shortestclerkline].ClerkTransLock);
    print("P_");
    print(itoa(ID));
    print(": Done with Clerk\n");
    Release(clerks[shortestclerkline].ClerkTransLock);

    /*7. get out - die die die( ;) muhahahhaha) */
    Acquire(hospitalLock);
    print("P_");
    print(itoa(ID));
    print(" Getting out of the hospital\n");
    peopleInHospital--;
    Release(hospitalLock);
}


