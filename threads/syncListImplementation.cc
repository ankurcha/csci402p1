#include "list.h"
#include "synchlist.h"

#define MINPATIENTS 20
#define MAXPATIENTS 100
#define MINDOCTORS 4
#define MAXDOCTORS 10
#define MINRCP 3
#define MAXRCP 1 

#define BUSY 0
#define FREE 1
#define SLEEPING 2

void Patient_func(int);
void Receptionist_func(int);
int signalsreceived;


    char *name;
    int i;

    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("patient_0");
    t->Fork((VoidFunctionPtr) Patient_func, 0);

        t = new Thread("patient_1");
    t->Fork((VoidFunctionPtr) Patient_func, 1);

     t = new Thread("patient_2");
    t->Fork((VoidFunctionPtr) Patient_func, 2);

    t = new Thread("receptionist_0");
    t->Fork((VoidFunctionPtr) Receptionist_func, 0);
}

struct Receptionist {
  int peopleInLine;
  int state;
  Condition *receptionCV;
  Lock *RecLock;
  int currentToken;
  
  Receptionist() {
    int ID = 0;
    state = FREE;
    peopleInLine = 0;
    line = new SynchList;
    char *recCV;
    sprintf(recCV, "%d_CV", ID);
    receptionCV = new Condition(recCV);
    sprintf(recCV, "%d_Lock", ID);
    RecLock = new Lock(recCV);
  }
  
  int getPeopleInLine() {
    return peopleInLine;
  }
  
  void setPeopleInLine(int people) {
    peopleInLine = people;
  }
  
  Condition * getReceptionCV() {
    return receptionCV;
  }
  
  Lock * getRecLock() {
    return RecLock;
  }
  
  int getCurrentToken() {
    return currentToken;
  }
  
  void setCurrentToken(int Token) {
    currentToken = Token;
  }
  
};

Receptionist receptionists[MAXRCP];
Condition *receptionBreak = new Condition("receptionBreakCV");
Lock *RecLineLock = new Lock("RecLineLock");

void Patient_func(int ID) {
    //Find the shortest Line------LOCK OUT ALL LINES----------
  printf("P_%d attempt acquiring RecLineLock\n",ID);
  RecLineLock->Acquire();
  printf("P_%d: Foo\n",ID);
  int indexShortestLine = 0;
  int len = 0;
  
    for (int i = 0; i < MAXRCP; i++) {
        if (len > receptionists[i].peopleInLine) {
            len = receptionists[i].peopleInLine;
            indexShortestLine = i;
        }
    }
  
    printf("P_%d: found shortest line R_%d len: %d\n",ID,indexShortestLine,len);
    //We got shortest length line, now wait in it
    if (len > 0) {
        //wait in line
        receptionists[indexShortestLine].peopleInLine++;
        printf("\nP_%d HELLOWORLD %d\n",ID,receptionists[indexShortestLine].peopleInLine);
	receptionists[indexShortestLine].getRecLock()->Acquire();
	RecLineLock->Release();
	receptionists[indexShortestLine].getReceptionCV()->Wait(receptionists[indexShortestLine].getRecLock());
	printf("P_%d: RecLineLock Released\n",ID);
    }else{
      printf("\nP_%d:Found Empty Line at receptionist# %d\n",ID,indexShortestLine);
      switch(receptionists[indexShortestLine].state){
	case FREE:
	case BUSY:
	case SLEEPING:
	  //Get into line because the manager is going to kick him
	  //back to life and patient will get serviced
          receptionists[indexShortestLine].peopleInLine++;
        printf("\nP_%d HELLOWORLD %d\n",ID,receptionists[indexShortestLine].peopleInLine);
	  receptionists[indexShortestLine].getRecLock()->Acquire();
	  RecLineLock->Release();
	  printf("P_%d: RecLineLock Released\n",ID);
	  printf("\nP_%d:Acquire recLock for R_%d, waiting on CV\n",ID,indexShortestLine);
	  receptionists[indexShortestLine].getReceptionCV()->Wait(receptionists[indexShortestLine].getRecLock());
	  break;
      }
    }

    printf("\n%d: Getting out of the line for receptionist# %d at position %d\n",ID,indexShortestLine,len);
    receptionists[indexShortestLine].peopleInLine--;
    printf("\nP_%d: Got out of line\n",ID);
    //    RecLineLock->Release();

    //--------------------NO MORE RecLineLock--------------
    //I go to the receptionist - not in line anymore, go to the counter
    //    receptionists[indexShortestLine].getRecLock()->Acquire();
    printf("\n%d: Signal the receptionist# %d",ID,indexShortestLine);
    receptionists[indexShortestLine].getReceptionCV()->Signal(receptionists[indexShortestLine].getRecLock());
      printf("P_%d Signal sent\n",ID);
 
    //Wait for the token
    printf("\n%d: Waiting for Receptionist# %d to give token \n",ID,indexShortestLine);
    receptionists[indexShortestLine].getReceptionCV()->Wait(receptionists[indexShortestLine].getRecLock());
    //Time to get token
    int myToken = receptionists[indexShortestLine].getCurrentToken();
    printf("\nPatient %d got token %d\n",ID,myToken);
    //Done with the receptionist,signal receptionist
    receptionists[indexShortestLine].getReceptionCV()->Signal(receptionists[indexShortestLine].getRecLock());
    printf("P_%d Signal sent\n",ID);
    receptionists[indexShortestLine].getRecLock()->Release();
    RecLineLock->Acquire();
    receptionBreak->Signal(RecLineLock); //Wake up receptionist in case he is sleeping
    RecLineLock->Release();
}

Lock *tokenLock = new Lock("tokenLock");
int currentToken = 0;

void Receptionist_func(int ID) {
  while (true) {
    RecLineLock->Acquire();
    receptionists[ID].state = FREE;
    //check my line length
    if (receptionists[ID].peopleInLine > 0) {
      //Someone in my line to get serviced
      printf("\nRecp %d: Found someone waiting in line",ID);
      receptionists[ID].getRecLock()->Acquire();
      receptionists[ID].getReceptionCV()->Signal(receptionists[ID].getRecLock());
      printf("R_%d Signal sent\n",ID);
    } else {
      printf("\nRecp %d: No one waiting in my line go to sleep:peopleinline=%d\n",ID,receptionists[ID].getPeopleInLine());
      //No one waiting for me, go on break - use CV wait()
      receptionists[ID].state = SLEEPING;
      //currentThread->Yield();
      receptionBreak->Wait(RecLineLock); //No one gets into line if
      //the receptionis is on
      //break, everyone just waits
      RecLineLock->Release();
      continue;
    }
        //Patient is coming to get a token Number, I must wait for them to
        //come up to the counter
        //receptionists[ID].getRecLock()->Acquire();
        RecLineLock->Release();
	//----------------NO MORE RecLineLock-----------------
	//Wait on the patient
	printf("\nRecp %d: Waiting for patient to come",ID);
        receptionists[ID].getReceptionCV()->Wait(receptionists[ID].getRecLock());
        receptionists[ID].state = BUSY;
	//I was woken up by a patient, get a token
        tokenLock->Acquire();
	printf("\nRecp %d: generating token",ID);
        int newToken = ++currentToken;
	printf("%d\n",newToken);
        tokenLock->Release();
        //Pass token to patient
        //Use the currentToken of the receptionist
        receptionists[ID].currentToken = newToken;
        //Signal Patient
	printf("\nRecp %d: signalling patient to take token",ID);
        receptionists[ID].getReceptionCV()->Signal(receptionists[ID].getRecLock());
      printf("R_%d Signal sent\n",ID);
        //Wait for the patient to get the token
	printf("\nRecp %d: waiting for patient to take token",ID);
        receptionists[ID].getReceptionCV()->Wait(receptionists[ID].getRecLock());
        //Patient has taken the lock
        //Time for another patient
        receptionists[ID].getRecLock()->Release();
    }//end of while(true)
}


void HospitalManager(int ID){
}
