#include "list.h"
#include "synchlist.h"

#define MINPATIENTS 20
#define MAXPATIENTS 100
#define MINDOCTORS 4
#define MAXDOCTORS 10
#define MINRCP 3
#define MAXRCP 5

void HospINIT() {
    Thread *t;
    char *name;
    int i;

    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("patient_0");
    t->Fork((VoidFunctionPtr) Patient, 0);

    t = new Thread("patient_1");
    t->Fork((VoidFunctionPtr) Patient, 1);

    t = new Thread("patient_2");
    t->Fork((VoidFunctionPtr) Patient, 2);

    t = new Thread("receptionist_0");
    t->Fork((VoidFunctionPtr) Receptionist, 0);


}

List **line;
int *lineLength;
Condition **receptionCV;
Lock *getInLineLock;


int ID;

struct Receptionist {
    int ID;
    int peopleInLine;
    SynchList *line;
    Condition *receptionCV;
    Lock *RecLock;
    int currentToken;

    Receptionist() {
        ID = 0;
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

Receptionist *receptionists = new Receptionist[MAXRCP];

Lock *RecLineLock = new Lock("RecLineLock");

void Patient(int ID) {
    //Find the shortest Line
    RecLineLock->Acquire();
    int indexShortestLine = 0;
    int len = 0;
    for (int i = 0; i < MAXRCP; i++) {
        if (len > receptionists[i].getPeopleInLine()) {
            len = receptionists[i].getPeopleInLine();
            indexShortestLine = i;
        }
    }
    //We got shortest length line, now wait in it
    if (len > 0) {
        //wait in line
        receptionists[indexShortestLine]->setPeopleInLine(receptionists[indexShortestLine]->getPeopleInLine() + 1);
        receptionists[indexShortestLine]->getReceptionCV()->Wait(RecLineLock);
    }
    receptionists[indexShortestLine]->setPeopleInLine(receptionists[indexShortestLine]->getPeopleInLine() - 1);
    RecLineLock->Release();
    //I go to the receptionist - not in line anymore
    receptionists[indexShortestLine]->getRecLock()->Acquire();
    receptionists[indexShortestLine]->getReceptionCV()->Signal(receptionists[indexShortestLine]->getRecLock());
    //Wait for the token
    receptionists[indexShortestLine]->getReceptionCV()->Wait(receptionists[indexShortestLine]->getRecLock());
    //Time to get token
    int myToken = receptionists[indexShortestLine]->getCurrentToken();
    //Done with the receptionist,signal receptionist
    receptionists[indexShortestLine]->getReceptionCV()->Signal(receptionists[indexShortestLine]);
    receptionists[indexShortestLine]->getRecLock()->Release();
}

Condition *receptionBreak = Condition("receptionBreakCV");
Lock *tokenLock = new Lock("tokenLock");
int currentToken = 0;

void receptionist(int ID) {
    while (true) {
        RecLineLock->Acquire();
        //check my line length
        if (receptionists[ID].getPeopleInLine() > 0) {
            //Someone in my line to get serviced
            receptionists[ID].getReceptionCV()->Signal(RecLineLock);
        } else {
            //No one waiting for me, go on break - use CV wait()
            receptionBreak->Wait(RecLineLock); //No one gets into line if
            //the receptionis is on
            //break, everyone just waits
            RecLineLock->Release();
            continue;
        }
        //Patient is coming to get a token Number, I must wait for them to
        //come up to the counter
        receptionists[ID].getRecLock()->Acquire();
        RecLineLock->Release();
        //Wait on the patient
        receptionists[ID].getReceptionCV()->Wait(receptionists[ID]->getRecLock());
        //I was woken up by a patient, get a token
        tokenLock->Acquire();
        int newToken = ++currentToken;
        tokenLock->Release();
        //Pass token to patient
        //Use the currentToken of the receptionist
        receptionists[ID]->setCurrentToken(newToken);
        //Signal Patient
        receptionists[ID]->getReceptionCV()->Signal(receptionists[ID]->getRecLock());
        //Wait for the patient to get the token
        receptionists[ID]->getReceptionCV()->Wait(receptionists[ID]->getRecLock());
        //Patient has taken the lock
        //Time for another patient
        receptionists[ID]->getRecLock()->Release();
    }//end of while(true)
}
