/*    Hospital Management Simulation */
/* */
/*    Patient Thread Function */
/*  */
/* CS 402 Fall 2009 */
/* Group 11 */
/*  Ankur Chauhan, ankurcha */
/*  Max Pflueger, pflueger */
/*  Aneesha Mathew, aneesham */

bool test_code2=true;
void patients(int ID){
    
    int myToken;
    int myDoctor;
    int myPrescription;

    /*//////////////////////////////////////////////// */
    /*//// Begin interaction with Receptionist /////// */
    /*//////////////////////////////////////////////// */

    Write("P:Attempt to acquire recpLineLock...\n",100, ConsoleOutput);
    Acquire(recpLineLock);
    Write("P:success\n",100,1);

    /* Find the shortest line */
    int shortestline = 0;
    int len = receptionists[0].peopleInLine;

    /*Find shortest Line */
    if (test4active == true) {
        Write("P:TEST4: Searching for the receptionist with the shortest line\n",100,1);
    }else {
        Write("P: Searching for the receptionist with the shortest line\n",100,1);
        
    }
    
    for (int i=0; i<numRecp; i++) {
        
        if (test4active == true) {
                /*Print the length of each line */
            Write("P:TEST4: Length of line at R_",100,1);
            Write(itoa(i),50,1);
            Write(" = ",3,1);
            Write(itoa(receptionists[i].peopleInLine),50,1);
            Write("\n",1,1);
        }
        
        if(receptionists[i].peopleInLine < len){
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    
    if (test4active == true) {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(":TEST4: Found shortest line with R_",100,1);
        Write(itoa(shortestline),50,1);Write(" len: ",100,1);Write(itoa(len),50,1);Write("\n",1,1);
    }else{
        Write("P_",2,1);Write(itoa(ID),50,1);Write(": Found shortest line with R_",100,1);
        Write(itoa(shortestline),50,1);Write(" len: ",10,1);Write(itoa(len),50,1);Write("\n",1,1);
    }

    /* Wait in line */
    receptionists[shortestline].peopleInLine++;
    receptionists[shortestline].receptionCV->Wait(recpLineLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" Got woken up, get out of line and going to counter for token\n",100,1);
    
    receptionists[shortestline].peopleInLine--;
    
    /*wait for the receptionist to prepare token for me, till then I wait */
    Release(recpLineLock);
    Acquire(receptionists[shortestline].transLock);

    /*token is ready just read it -- print it out in our case */
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Reading Token..\n",20,1);
    myToken = receptionists[shortestline].currentToken;
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": My token is ",50,1);
    Write(itoa(ID),50,1);
    Write("..yeah!!\n",50,1);

    /*Done, signal receptionist that he can proceed  */
    Signal(receptionists[shortestline].receptionistWaitCV, receptionists[shortestline].transLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Signal receptionist R_",100,1);
    Write(itoa(shortestline),50,1);
    Write(" to continue, I am done\n",100,1);

    /*Release transaction lock */
    Release(receptionists[shortestline].transLock);
    
    /*///////////////////////////////////////////////// */
    /*///// Interaction with Doctor and Doorboy /////// */
    /*///////////////////////////////////////////////// */

    /*Calculate which doctor I want to see */

    myDoctor = (int)(Random()) % numDoctors;
    if(test2active==true)
	{
	    Write("P_",2,1);
            Write(itoa(ID),50,1);
            Write(" :TEST2: Going to meet doctor D_",100,1);
            Write(itoa(myDoctor),50,1);Write("\n",1,1);
	}
	 if(test7active==true)
	{
	    Write("P_",2,1);
            Write(itoa(ID),50,1);
            Write(" :TEST7: Waiting in doctor D_",100,1);
            Write(itoa(myDoctor),50,1);
           Write(" Queue\n",10,1);
	}else{
            Write("P_",1,1);
            Write(itoa(ID),50,1);
            Write(" : Going to meet doctor D_",100,1);
            Write("\n",1,1);
        }

    /* Acquire doc's line lock */
    Acquire(doctors[myDoctor].LineLock);

    /* Wait on the line -- to be woken up by the doorboy */
    if(test2active==true)
	{
            Write("P_",1,1);
            Write(itoa(ID),50,1);
	    Write(" :TEST2: Join line and Waiting for doorboy to tell me to go\n",100,1);
            Write("\n",1,1);
	}
	else{
            Write("P_",1,1);
            Write(itoa(ID),50,1);
	    Write(" : Join line and Waiting for doorboy to tell me to go\n",100,1);
            Write("\n",1,1);
        }
    doctors[myDoctor].peopleInLine++;
    Wait(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);
    Write("P_",3,1);
    Write(itoa(ID),50,1);
    Write(" : Doorboy told me to go to doctor, proceeding....\n",100,1);
    doctors[myDoctor].peopleInLine--;

    /* move into the doctor's transaction lock */
    Release(doctors[myDoctor].LineLock);
      
    
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" : Trying to acquire doctor's translock\n",100,1);
   
    Acquire(doctors[myDoctor].transLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" : Success\n",50,1);

    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" : Giving doctor the token...\n",100,1);
    /*The doctor is waiting for me to provide my info, oblige him!! */
    doctors[myDoctor].patientToken = myToken;

    /* hand off to the doctor thread for consultation */
     if(test7active==true)
	{
	    Write("P_",2,1);
            Write(itoa(ID),50,1);
            Write(" :TEST7: Consulting Doctor D_",100,1);
            Write(itoa(myDoctor),50,1);
            Write(" now...\n",20,1);
				
	}
	else{
            Write("P_",3,1);
            Write(itoa(ID),50,1);
            Write(" : Consulting Doctor D_",100,1);
            Write(itoa(myDoctor),50,1);
            Write(" now...\n",20,1);
        }
    Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);

    /* Consultation finished, now I have to get the prescription from the doctor */
    myPrescription = doctors[myDoctor].prescription;
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" : Consultation finished and  Got prescription ",100,1);
    Write(itoa(myPrescription),50,1);
    Write("\n",1,1);

    /* Signal the doctor that I have taken the prescription and left */
    Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
    Release(doctors[myDoctor].transLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Done with Doctor, going to cashier.\n",50,1);
    /*////////////////////////////////////////// */
    /*///////  Interaction with Cashier //////// */
    /*////////////////////////////////////////// */
    
    Acquire(cashierLineLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Acquiring cashierLineLock\n",100,1);
    /* find the shortest line */
    int myCashier = 0;
    int sLen = cashiers[0].lineLength;
    if (test4active == true) {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(":TEST4: Finding shortest Line of cashiers\n",100,1);
    }else {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(": Finding shortest Line of cashiers\n",100,1);
    }

    for(int i=1; i < numCashiers; ++i) {
        if (test4active == true) {
                /*Print the length of each line */
            Write("P_",2,1);
            Write(itoa(ID),50,1);
            Write(":TEST4: Length of line at R_",50,1);
            Write(itoa(i),50,1);
            Write(" = ",3,1);
            Write(itoa(cashiers[i].lineLength),50,1);
            Write("\n",1,1);
        }
        
        if(cashiers[i].lineLength < sLen) {
            myCashier = i;
            sLen = cashiers[i].lineLength;
        }
    }
    
    if (test4active == true) {
        Write("P_",3,1);
        Write(itoa(ID),50,1);
        Write(":TEST4: Found shortest cashier line C_",100,1);
        Write(myCashier,50,1);
        Write(itoa(myCashier),50,1);
        Write("len: ",10,1);
        Write(itoa(sLen),50,1);
        Write("\n",100,1);
    }else {
        Write("P_%d: Found shortest cashier line C_%d len: %d\n",
               ID,myCashier,sLen);
    }

    /*if(sLen > 0) {get in line} else {get in line} */
    /* there are a lot of cases here, but they all result in us getting in line */
    cashiers[myCashier].lineLength ++;
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Waiting in line for cashier C_",100,1);
    Write(itoa(myCashier),50,1);
    Write(" to attend to me, Line length: ",100,1);
    Write(itoa(cashiers[myCashier].lineLength),50,1);
    Write("\n",1,1);
    Wait(cashiers[myCashier].lineCV,cashierLineLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Going to meet cashier C_",100,1);
    Write(itoa(myCashier),50,1);
    Write("\n",1,1);
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
    Write( "P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Paying money.\n",50,1);
    /* done */
    Signal(cashiers[myCashier].transCV, cashiers[myCashier].transLock);
    Release(cashiers[myCashier].transLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Done with cashier\n",100,1);

    /*//////////////////////////////////////////////// */
    /*//////  Interaction with Pharmacy Clerk //////// */
    /*//////////////////////////////////////////////// */
    
    Write("P_",3,1);
    Write(itoa(ID),50,1);
    Write(":Attempt to acquire ClerkLinesLock...\n",100,1);
    Acquire(ClerkLinesLock)
    Write("success\n",100,1);
    int shortestclerkline = 0;
    int length = clerks[0].patientsInLine;
    if (test4active == true) {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(":TEST4: Finding shortest Line of clerks\n",100,1);
    }else {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(": Finding shortest Line of clerks\n",100,1);
    }
    
    /*Find shortest Line */
    for (int i=0; i<numClerks; i++) {
        if (test4active == true) {
                /*Print the length of each line */
            Write("P_",2,1);
            Write(itoa(ID),50,1);
            Write(": TEST4: Length of line at CL_",100,1);
            Write(itoa(i),50,1);
            Write(" = ",3,1);
            Write(itoa(clerks[i].patientsInLine),50,1);
            Write("\n",1,1);
        }
        if(clerks[i].patientsInLine < length){
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }

    
    if (test4active == false) {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(": Found shortest pharmacy clerk line CL_",100,1);
        Write(itoa(shortestclerkline),50,1);
        Write(" len: ",10,1);
        Write(itoa(length),50,1);
        Write("\n",1,1);
    }else {
        Write("P_",2,1);
        Write(itoa(ID),50,1);
        Write(":TEST4: Found shortest pharmacy clerk line CL_",100,1);
        Write(itoa(shortestclerkline),50,1);
        Write(" len: ",7,1);
        Write(itoa(length),50,1);
        Write("\n",1,1);
    }

        /*wait in line for my turn */
    clerks[shortestclerkline].patientsInLine++;
    cout << "P_"<<ID<<": Waiting in line for clerk CL_"<<shortestclerkline
    <<" to attend to me, Line length: "<<clerks[shortestclerkline].patientsInLine<<"\n";
    
    Wait(clerks[shortestclerkline].ClerkCV, ClerkLinesLock);
    
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" Got woken up, got out of line and going to the Pharmacy CLerk to give prescription.\n",150,1);
    clerks[shortestclerkline].patientsInLine--;
    
    Release(ClerkLinesLock);
    Acquire(clerks[shortestclerkline].ClerkTransLock);
        /*signal ParmacyClerk that i am ready to give Prescription */
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Acquired ClerkTransLock\n",100,1);
        /*Entered the line no need to hold all lines others may now continue */
    /*wait for the PharmacyClerk to Get the prescription from me.. so I wait */
     clerks[shortestclerkline].patPrescription = myPrescription;
    Write( "P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Gave prescriptiong, waiting for medicines.\n",100,1);;
    /* wait for clerk to give cost */
    Signal(clerks[shortestclerkline].ClerkTransCV, clerks[shortestclerkline].ClerkTransLock);
    Wait(clerks[shortestclerkline].ClerkTransCV, clerks[shortestclerkline].ClerkTransLock);

    /* provide the money */
    Write( "P_",2,1);
    Write(itoa(ID),50,1);
    Write(": Got Medicines, making payment.\n",50,1);
    clerks[shortestclerkline].payment = clerks[shortestclerkline].fee;

    /* done */
    Signal(clerks[shortestclerkline].ClerkTransCV,clerks[shortestclerkline].ClerkTransLock);
    Write("P_",3,1);
    Write(itoa(ID),50,1);
    Write(": Done with Clerk\n",50,1);
    Release(clerks[shortestclerkline].ClerkTransLock);

    /*7. get out - die die die( ;) muhahahhaha) */
    Acquire(hospitalLock);
    Write("P_",2,1);
    Write(itoa(ID),50,1);
    Write(" Getting out of the hospital\n",100,1);
    peopleInHospital--;
    Release(hospitalLock);
}


