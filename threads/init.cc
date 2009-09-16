/*
 *  init.cpp
 *  
 *
 *  Created by Ankur Chauhan on 9/15/09.
 *
 */

#define BUSY 0
#define FREE 1
#define SLEEPING 2

struct Receptionists{
    int peopleInLine;
    int state;
    Condition *receptionCV;
    Lock *LineLock;
    int currentToken;
    Lock *receptionistWaitLock;
    Condition *receptionistWaitCV;
    
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

int MAX_DOCTORS;
int RECP_MAX;
int MAX_PATIENTS;

Receptionists receptionists[3];
int TokenCounter;

Lock *AllLinesLock = new Lock("AllLineLock");
Lock *TokenCounterLock = new Lock("TokenCounterLock");

void patients(int ID){
    int myToken;
    int myDoctor;
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
    //1. Acquire doc's line lock
    //2. Wait on the line -- to be woken up by the bell boy
    //3. Wait for the doctor to wake you up
    //4. get Consulted
    //5. goto cashier similar to 1 - 5
    //6. goto pharmacy  1 - 5
    //7. get out - die die die( ;) )
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
    
}

void HospINIT() {
    
    MAX_DOCTORS = 5;
    RECP_MAX = 1;
    MAX_PATIENTS = 3;
    
    INIT(); // initiate threads
}

void hospitalManager(int ID){
    int sleeptime = Random() % 300;
    while (true) {
        //Sleep for some random amount of time
        while (sleeptime > 0) {
            currentThread->Yield();
        }
        //I am on rounds now, Time to kick some ass
        for (int i=0; i<RECP_MAX; i++) {//Check for waiting patients
            if (receptionists[i].peopleInLine > 0 && receptionists[i].state == SLEEPING) {
                //Wake up this receptionist up
                receptionists[ID].ReceptionistBreakLock->Acquire();
                receptionists[ID].ReceptionistBreakCV->Broadcast(receptionists[ID].ReceptionistBreakLock);
                receptionists[ID].ReceptionistBreakLock->Release();
                
            }
        }
    }
}
