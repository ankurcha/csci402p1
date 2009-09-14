#include "synchlist.h"

struct Patient{
  int tokenNumber;
  Thread *owner;
};

SynchList *processList = new SynchList;
Lock getTokenLock("Get Token");
Condition *getTokenLockCV = new Condition("Get Token CV");

Lock waitLock("WaitLock");
Condition *waitCV = new Condition("Wait CV");

int tokenCounter = 0;

void patient(int id){
  Patient *me = new Patient();
  me->owner = currentThread;
  processList->Append((void *)me);
  printf("\n%s:Adding to synchList\n",currentThread->getName());
  waitLock.Acquire();
  waitCV->Wait(&waitLock);
  printf("%s Awake\n",currentThread->getName());
  printf("\n%s: %d\n",currentThread->getName(),me->tokenNumber);
  waitLock.Release();
}



void receptionist(int id){
  Patient *target = (Patient *)processList->Remove();
  //Increment Token counter
  getTokenLock.Acquire();
  tokenCounter++;
  getTokenLock.Release();
  target->tokenNumber = tokenCounter;
  waitCV->Signal(&waitLock);
}

void startTest(){
  Thread *t;

  printf("\nStartTest\n");

  t = new Thread("patient_0");
  t->Fork((VoidFunctionPtr) patient, 0);

  t = new Thread("patient_1");
  t->Fork((VoidFunctionPtr) patient, 1);

  t = new Thread("patient_2");
  t->Fork((VoidFunctionPtr) patient, 2);


  t = new Thread("receptionist_0");
  t->Fork((VoidFunctionPtr) receptionist, 0);

}
