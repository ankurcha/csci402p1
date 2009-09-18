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

void patients(int ID){
    int myToken;
    int myDoctor;
    int myPrescription;

    //////////////////////////////////////////////////
    ////// Begin interaction with Receptionist ///////
    //////////////////////////////////////////////////

    printf("P_%d:Attempt to acquire AllLinesLock...",ID);
    AllLinesLock->Acquire();
    printf("success\n");
    int shortestline = 0;
    int len = receptionists[0].peopleInLine;
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
        AllLinesLock->Release();
        receptionists[shortestline].LineLock->Acquire();
        receptionists[shortestline].receptionistWaitCV->
                    Wait(receptionists[shortestline].LineLock);
        printf("P_%d: AllLinesLock Released, Now Waiting for signal by " 
                + "Receptionist\n",ID);
    }else { //No one else in line
        switch (receptionists[shortestline].state) {
            case FREE:
            case BUSY:
            case SLEEPING:
                //wait in line
                receptionists[shortestline].peopleInLine++;
                //Entered the line no need to hold all lines others may now continue
                AllLinesLock->Release();
                receptionists[shortestline].LineLock->Acquire();
                receptionists[shortestline].receptionistWaitCV->
                            Wait(receptionists[shortestline].LineLock);
                printf("P_%d: AllLinesLock Released, Now Waiting for signal "
                        + "by Receptionist\n",ID);
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
    printf("P_%d: My token is %d..yeah!!\n",ID,myToken);
    //Done, signal receptionist that he can proceed 
    receptionists[shortestline].receptionistWaitCV->
                Signal(receptionists[shortestline].LineLock);
    //Release Line Lock
    receptionists[shortestline].LineLock->Release();
    
    ///////////////////////////////////////////////////
    /////// Interaction with Doctor and Doorboy ///////
    ///////////////////////////////////////////////////

    //Calculate which doctor I want to see
    myDoctor = Random() % MAX_DOCTORS;
    printf("P_%d : Going to meet doctor D_%d\n",ID,myDoctor);
    //1. Acquire doc's line lock
    doorboys[myDoctor].LineLock->Acquire();
    //2. Wait on the line -- to be woken up by the bell boy
    printf("P_%d : Waiting for doorboy to tell me to go\n",ID);
        //Add to the number of people waiting on the line
    doorboys[myDoctor].peopleInLine++;
    doorboys[myDoctor].LineCV->Wait(doorboys[myDoctor].LineLock);
    //doctor told the door boy to wake me up for consultation, he is waiting 
    // for me to respond
    //Now I have to provide my token numeber to the doctor as he is ready for 
    // me, I must acquire lock for that and then provide all the information 
    // befor i proceed
    doctors[myDoctor].patientRespondLock->Acquire();
    //I can release the line lock so that other people may also join in
    //Also I should decrement the number of people in the line, as I am 
    // getting out of the line
    doorboys[myDoctor].peopleInLine--;
        //Now release the lock on the line and enter the consultation room
    doorboys[myDoctor].LineLock->Release();
    //The doctor is waiting for me to provide my info, oblige him!!
    printf("P_%d : Consulting Doctor D_%d now...\n",ID,myDoctor);
    doctors[myDoctor].currentPatientToken = myToken;
    doctors[myDoctor].patientRespondCV->Wait(doctors[myDoctor].patientRespondLock);
    //Consultation in process....
    //4. Consultation finished, now I have to get the prescription from the doctor
    //The doctor would be waiting for me to take this
    printf("P_%d : Consultation finished!!\n",ID);
    doctors[myDoctor].patientPrescriptionLock->Acquire();
        //Take prescription form the doctor
    myPrescription = doctors[myDoctor].currentPrescription;
    printf("P_%d : Got prescription# %d\n",ID,myPrescription);
    //Signal the doctor that I have taken the prescription
    doctors[myDoctor].patientPrescriptionCV->
                Signal(doctors[myDoctor].patientPrescriptionLock);
    doctors[myDoctor].patientRespondLock->Release();

    ////////////////////////////////////////////
    /////////  Interaction with Cashier ////////
    ////////////////////////////////////////////
    
    cashierLineLock->Acquire();
    
    // find the shortest line
    int myCashier = 0;
    int sLen = cashiers[0].lineLength;
    for(int i=1; i < numCashiers; ++i) {
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
    printf("success\n");
    int shortestclerkline = 0;
    int length = 0;
    //Find shortest Line
    for (int i=0; i<MAX_CLERKS; i++) {
        if(clerks[i].patientsInLine < length){
            length = clerks[i].patientsInLine;
            shortestclerkline = i;
        }
    }
    printf("P_%d: found shortest line PC_%d len: %d\n",ID,shortestclerkline,length);
    if (length >0) {
        //wait in line for my turn
        clerks[shortestclerkline].patientsInLine++;
        clerks[shortestclerkline].ClerkTransLock->Acquire();
        ClerkLinesLock->Release();
        clerks[shortestclerkline].ClerkCV->
                    Wait(clerks[shortestclerkline].ClerkTransLock);
        printf("P_%d: ClerkLinesLock Released, Now Waiting for signal by "
                + "PharmacyClerk\n",ID);
    }else { //No one else in line
        switch (clerks[shortestclerkline].state) {
            case FREE: 
            case BUSY:
            case SLEEPING:
                //wait in line
                clerks[shortestclerkline].patientsInLine++;
                clerks[shortestclerkline].ClerkCV->Wait(clerks[shortestclerkline].ClerkTransLock);
                printf("P_%d: ClerkLinesLock Released, Now Waiting for signal "
                        + "by CLerk\n",ID);
                break;
            default:
                break;
        } 
    }
    
    printf("P_%d Got woken up, got out of line and going to the PHarmacy "
            + "CLerk to give prescription\n",ID);
    clerks[shortestclerkline].patientsInLine--;
    //signal ParmacyClerk that i am ready to give Prescription
    clerks[shortestclerkline].ClerkTransLock->Acquire();
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
}


