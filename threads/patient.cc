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
 
    for (int i=0; i<numRecp; i++) {

        if(receptionists[i].peopleInLine < len){
            len = receptionists[i].peopleInLine;
            shortestline = i;
        }
    }
    printf("P_%d: found shortest line R_%d len: %d\n",ID,shortestline,len);

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
    printf("P_%d : Going to meet doctor D_%d\n",ID,myDoctor);

    // Acquire doc's line lock
    doctors[myDoctor].LineLock->Acquire();

    // Wait on the line -- to be woken up by the doorboy
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

    ////////////////////////////////////////////
    /////////  Interaction with Cashier ////////
    ////////////////////////////////////////////
    
    cashierLineLock->Acquire();
    // find the shortest line
    int myCashier = 0;
    int sLen = cashiers[0].lineLength;
    for(int i=1; i < numPatients; ++i) {
        if(cashiers[i].lineLength < sLen) {
            myCashier = i;
            sLen = cashiers[i].lineLength;
        }
    }

    //if(sLen > 0) {get in line} else {get in line}
    // there are a lot of cases here, but they all result in us getting in line
    cashiers[myCashier].lineLength ++;
    cashiers[myCashier].lineCV->Wait(cashierLineLock);
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

    // done
    cashiers[myCashier].transCV->Signal(cashiers[myCashier].transLock);
    cashiers[myCashier].transLock->Release();

    //////////////////////////////////////////////////
    ////////  Interaction with Pharmacy Clerk ////////
    //////////////////////////////////////////////////
    
    printf("P_%d:Attempt to acquire ClerkLinesLock...",ID);
    ClerkLinesLock->Acquire();
    int shortestclerkline = clerks[0].patientsInLine;
    int length = 0;
    //Find shortest Line
    for (int i=0; i<numClerks; i++) {
        if(clerks[i].patientsInLine < length){
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }
    printf("P_%d: found shortest line C_%d len: %d\n",ID,shortestclerkline,length);
    clerks[shortestclerkline].patientsInLine++;
    clerks[shortestclerkline].ClerkTransLock->Acquire();
    clerks[shortestclerkline].ClerkCV->
                    Wait(clerks[shortestclerkline].ClerkTransLock);

    cout<<"P_"<<ID<<" Got woken up, got out of line and going to the PHarmacy "
        <<"Clerk to give prescription.\n";
    clerks[shortestclerkline].patientsInLine--;
    //signal ParmacyClerk that i am ready to give Prescription
    //clerks[shortestclerkline].ClerkTransLock->Acquire();
                //Entered the line no need to hold all lines others may now continue
     ClerkLinesLock->Release();
    //wait for the PharmacyClerk to Get the prescription from me.. so I wait
     clerks[shortestclerkline].patPrescription = myPrescription;

    // wait for clerk to give cost
    clerks[shortestclerkline].ClerkTransCV->
                Signal(clerks[shortestclerkline].ClerkTransLock);
    clerks[shortestclerkline].ClerkTransCV->
                Wait(clerks[shortestclerkline].ClerkTransLock);

    // provide the money
    
    clerks[shortestclerkline].payment = clerks[shortestclerkline].fee;

    // done
    clerks[shortestclerkline].ClerkTransCV->
                Signal(clerks[shortestclerkline].ClerkTransLock);
    clerks[shortestclerkline].ClerkTransLock->Release();

    //7. get out - die die die( ;) )
    hospitalLock->Acquire();
    cout<<"P_"<<ID<<" Getting out of the hospital\n";
    peopleInHospital--;
    hospitalLock->Release();
}


