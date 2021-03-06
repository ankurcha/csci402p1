// patient.cc
//    Hospital Management Simulation
//
//    Patient Thread Function
// 
// CS 402 Fall 2009
// Group 11
//  Ankur Chauhan, ankurcha
//  Max Pflueger, pflueger
//  Aneesha Mathew, aneesham

using namespace std;
bool test_code2=true;
void patients(int ID){
    
    int myToken;
    int myDoctor;
    int myPrescription;

    //////////////////////////////////////////////////
    ////// Begin interaction with Receptionist ///////
    //////////////////////////////////////////////////

    printf("P_%d:Attempt to acquire recpLineLock...\n",ID);
    recpLineLock->Acquire();
    printf("P_%d:success\n",ID);

    // Find the shortest line
    int shortestline = 0;
    int len = receptionists[0].peopleInLine;

    //Find shortest Line
    if (test4active == true) {
        cout << "P_"<<ID
        <<":TEST4: Searching for the receptionist with the shortest line\n";
    }else {
        cout << "P_"<<ID
        <<": Searching for the receptionist with the shortest line\n";
        
    }
    
    for (int i=0; i<numRecp; i++) {
        
        if (test4active == true) {
                //Print the length of each line
            cout <<"P_"<<ID<<":TEST4: Length of line at R_"<<i<<" = "
            <<receptionists[i].peopleInLine<<endl;
        }
        
        if(receptionists[i].peopleInLine < len){
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    
    if (test4active == true) {
        printf("P_%d:TEST4: Found shortest line with R_%d len: %d\n",
               ID,shortestline,len);
    }else{
        printf("P_%d: Found shortest line with R_%d len: %d\n",
               ID,shortestline,len);
    }

    // Wait in line
    receptionists[shortestline].peopleInLine++;
    receptionists[shortestline].receptionCV->Wait(recpLineLock);
    printf("P_%d Got woken up, get out of line and going to counter for token\n",ID);
    receptionists[shortestline].peopleInLine--;
    
    //wait for the receptionist to prepare token for me, till then I wait
    recpLineLock->Release();
    receptionists[shortestline].transLock->Acquire();

    //token is ready just read it -- print it out in our case
    printf("P_%d: Reading Token..\n",ID);
    myToken = receptionists[shortestline].currentToken;
    printf("P_%d: My token is %d..yeah!!\n",ID,myToken);

    //Done, signal receptionist that he can proceed 
    receptionists[shortestline].receptionistWaitCV->
                Signal(receptionists[shortestline].transLock);
    printf("P_%d: Signal receptionist R_%d to continue, I am done\n",
            ID,shortestline);

    //Release transaction lock
    receptionists[shortestline].transLock->Release();
    
    ///////////////////////////////////////////////////
    /////// Interaction with Doctor and Doorboy ///////
    ///////////////////////////////////////////////////

    //Calculate which doctor I want to see

    myDoctor = (int)(Random()) % numDoctors;
    if(test2active==true)
	{
				printf("P_%d :TEST2: Going to meet doctor D_%d\n",ID,myDoctor);
	}
	 if(test7active==true)
	{
				printf("P_%d :TEST7: Waiting in doctor D_%d Queue\n",ID,myDoctor);
	}
	
	else
    printf("P_%d : Going to meet doctor D_%d\n",ID,myDoctor);
    

    // Acquire doc's line lock
    doctors[myDoctor].LineLock->Acquire();

    // Wait on the line -- to be woken up by the doorboy
    if(test2active==true)
	{
				printf("P_%d :TEST2: Join line and Waiting for doorboy to tell me to go\n",ID);
	}
	else
    printf("P_%d : Join line and Waiting for doorboy to tell me to go\n",ID);
    
    doctors[myDoctor].peopleInLine++;
    doctors[myDoctor].LineCV->Wait(doctors[myDoctor].LineLock);
    printf("P_%d : Doorboy told me to go to doctor, proceeding....\n",ID);
    doctors[myDoctor].peopleInLine--;

    // move into the doctor's transaction lock
    doctors[myDoctor].LineLock->Release();
      
    
    printf("P_%d : Trying to acquire doctor's translock\n",ID);
   
    doctors[myDoctor].transLock->Acquire();
    printf("P_%d : Success\n",ID);

    printf("P_%d : Giving doctor the token...\n",ID);
    //The doctor is waiting for me to provide my info, oblige him!!
    doctors[myDoctor].patientToken = myToken;

    // hand off to the doctor thread for consultation
     if(test7active==true)
	{
				printf("P_%d :TEST7: Consulting Doctor D_%d now...\n",ID,myDoctor);
				
	}
	else
    printf("P_%d : Consulting Doctor D_%d now...\n",ID,myDoctor);
    doctors[myDoctor].transCV->Signal(doctors[myDoctor].transLock);
    doctors[myDoctor].transCV->Wait(doctors[myDoctor].transLock);

    // Consultation finished, now I have to get the prescription from the doctor
    myPrescription = doctors[myDoctor].prescription;
    printf("P_%d : Consultation finished and  Got prescription %d\n", 
            ID, myPrescription);

    // Signal the doctor that I have taken the prescription and left
    doctors[myDoctor].transCV->Signal(doctors[myDoctor].transLock);
    doctors[myDoctor].transLock->Release();
    cout << "P_"<<ID<<": Done with Doctor, going to cashier.\n";
    ////////////////////////////////////////////
    /////////  Interaction with Cashier ////////
    ////////////////////////////////////////////
    
    cashierLineLock->Acquire();
    cout << "P_"<<ID<<": Acquiring cashierLineLock\n";
    // find the shortest line
    int myCashier = 0;
    int sLen = cashiers[0].lineLength;
    if (test4active == true) {
        cout <<"P_"<<ID<<":TEST4: Finding shortest Line of cashiers\n";
    }else {
        cout <<"P_"<<ID<<": Finding shortest Line of cashiers\n";
    }

    for(int i=1; i < numCashiers; ++i) {
        if (test4active == true) {
                //Print the length of each line
            cout <<"P_"<<ID<<":TEST4: Length of line at R_"<<i<<" = "
            <<cashiers[i].lineLength<<endl;
        }
        
        if(cashiers[i].lineLength < sLen) {
            myCashier = i;
            sLen = cashiers[i].lineLength;
        }
    }
    
    if (test4active == true) {
        printf("P_%d:TEST4: Found shortest cashier line C_%d len: %d\n",
               ID,myCashier,sLen);
    }else {
        printf("P_%d: Found shortest cashier line C_%d len: %d\n",
               ID,myCashier,sLen);
    }

    //if(sLen > 0) {get in line} else {get in line}
    // there are a lot of cases here, but they all result in us getting in line
    cashiers[myCashier].lineLength ++;
    cout << "P_"<<ID<<": Waiting in line for cashier C_"<<myCashier
    <<" to attend to me, Line length: "<<cashiers[myCashier].lineLength<<"\n";
    cashiers[myCashier].lineCV->Wait(cashierLineLock);
    cout << "P_"<<ID<<": Going to meet cashier C_"<<myCashier<<"\n";
    cashiers[myCashier].lineLength --;

    //// APPROACH THE DESK ////
    cashierLineLock->Release();
    cashiers[myCashier].transLock->Acquire();

    // provide token to cashier
    cashiers[myCashier].patToken = myToken;

    // wait for cashier to come back with the fee
    cashiers[myCashier].transCV->Signal(cashiers[myCashier].transLock);
    cashiers[myCashier].transCV->Wait(cashiers[myCashier].transLock);

    // provide the money
    cashiers[myCashier].payment = cashiers[myCashier].fee;
    cout << "P_"<<ID<<": Paying money.\n";
    // done
    cashiers[myCashier].transCV->Signal(cashiers[myCashier].transLock);
    cashiers[myCashier].transLock->Release();
    cout << "P_"<<ID<<": Done with cashier\n";

    //////////////////////////////////////////////////
    ////////  Interaction with Pharmacy Clerk ////////
    //////////////////////////////////////////////////
    
    printf("P_%d:Attempt to acquire ClerkLinesLock...\n",ID);
    ClerkLinesLock->Acquire();
    printf("success\n");
    int shortestclerkline = 0;
    int length = clerks[0].patientsInLine;
    if (test4active == true) {
        cout <<"P_"<<ID<<":TEST4: Finding shortest Line of clerks\n";
    }else {
        cout <<"P_"<<ID<<": Finding shortest Line of clerks\n";
    }
    
    //Find shortest Line
    for (int i=0; i<numClerks; i++) {
        if (test4active == true) {
                //Print the length of each line
            cout <<"P_"<<ID<<": TEST4: Length of line at CL_"<<i<<" = "
            <<clerks[i].patientsInLine<<endl;
        }
        if(clerks[i].patientsInLine < length){
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }

    
    if (test4active == false) {
        printf("P_%d: Found shortest pharmacy clerk line CL_%d len: %d\n",
               ID,shortestclerkline,length);
    }else {
        printf("P_%d:TEST4: Found shortest pharmacy clerk line CL_%d len: %d\n",
               ID,shortestclerkline,length);
    }

        //wait in line for my turn
    clerks[shortestclerkline].patientsInLine++;
    cout << "P_"<<ID<<": Waiting in line for clerk CL_"<<shortestclerkline
    <<" to attend to me, Line length: "<<clerks[shortestclerkline].patientsInLine<<"\n";
    
    clerks[shortestclerkline].ClerkCV->Wait(ClerkLinesLock);
    
    cout<<"P_"<<ID<<" Got woken up, got out of line and going to the Pharmacy "

        <<"CLerk to give prescription.\n";
    clerks[shortestclerkline].patientsInLine--;
    
    ClerkLinesLock->Release();
    clerks[shortestclerkline].ClerkTransLock->Acquire();
        //signal ParmacyClerk that i am ready to give Prescription
    cout << "P_"<<ID<<": Acquired ClerkTransLock\n";
        //Entered the line no need to hold all lines others may now continue
    //wait for the PharmacyClerk to Get the prescription from me.. so I wait
     clerks[shortestclerkline].patPrescription = myPrescription;
    cout << "P_"<<ID<<": Gave prescriptiong, waiting for medicines.\n";
    // wait for clerk to give cost
    clerks[shortestclerkline].ClerkTransCV->
                Signal(clerks[shortestclerkline].ClerkTransLock);
    clerks[shortestclerkline].ClerkTransCV->
                Wait(clerks[shortestclerkline].ClerkTransLock);

    // provide the money
    cout << "P_"<<ID<<": Got Medicines, making payment.\n";
    clerks[shortestclerkline].payment = clerks[shortestclerkline].fee;

    // done
    clerks[shortestclerkline].ClerkTransCV->
                Signal(clerks[shortestclerkline].ClerkTransLock);
    cout << "P_"<<ID<<": Done with Clerk\n";
    clerks[shortestclerkline].ClerkTransLock->Release();

    //7. get out - die die die( ;) )
    hospitalLock->Acquire();
    cout<<"P_"<<ID<<" Getting out of the hospital\n";
    peopleInHospital--;
    hospitalLock->Release();
}


