// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    }
    value--; 					// semaphore available,
						// consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
#ifdef CHANGED
  name = debugName;
  owner = NULL;
  islocked = false;
  queue = new List;
  numWaiting = 0;
#endif
}

Lock::~Lock() {
#ifdef CHANGED
  delete queue;
#endif
}

void Lock::Acquire()
{
#ifdef CHANGED
    //disable interrupts to make this atomic
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // do nothing if I already own this lock
    if(owner == currentThread) {
        printf("thread attempted to reacquire lock %s\n", name);
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    while(islocked) {
        // put ourselves on the queue and sleep
        queue->Append((void*) currentThread);
        numWaiting++;
        currentThread->Sleep();

        // we awake!  break the loop if we own the lock
        if(owner == currentThread) {
	    break;
        } else {
            // this thread should not wake without owning the lock,
            //  but just in case
            printf("thread waiting on lock %s was woken without ownership\n", name);
        }
    }
    // the above loop exited, so the lock is ours (mwa ha ha)

    islocked = true;
    owner = currentThread;
    
    (void) interrupt->SetLevel(oldLevel);
    // we are done, interrupts are back to previous status
#endif
}

void Lock::Release()
{
#ifdef CHANGED
    // disable interrupts to make this atomic
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // check that we own the lock we wish to Release
    if(owner != currentThread) {
        printf("ERROR: thread other than owner tried to Release lock %s\n", name);
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    //DEBUG('t', "owner == currentThread");

    // if someone is waiting on this lock, give it to them
    if(numWaiting > 0) {
        Thread* thread = (Thread *)queue->Remove();
        numWaiting--;
        if(thread != NULL) {
	    owner = thread;
            scheduler->ReadyToRun(thread);
        } else {
            printf("ERROR: NULL thread waiting for lock %s!\n", name);
        }
    } else { // no one is waiting, open the lock
        islocked = false;
        owner = NULL;
    }

    (void) interrupt->SetLevel(oldLevel);
#endif
}

bool Lock::isHeldByCurrentThread()
{
#ifdef CHANGED
    if( owner == currentThread )
        return true;
    else
        return false;
#endif
}

Condition::Condition(char* debugName) {
#ifdef CHANGED
  name = debugName;
  CVLock = NULL;
  queue = new List;
#endif
}

Condition::~Condition() {
#ifdef CHANGED
  delete queue;
#endif
}

void Condition::Wait(Lock* conditionLock) {
#ifdef CHANGED
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // check that this thread owns the lock, boot the cheaters
    if(!conditionLock->isHeldByCurrentThread()){
        printf("Lock not held by thread, Wait failed on CV %s", name);
        return;
    }

    // Store Lock for the First Waiter
    if(CVLock == NULL){
        CVLock = conditionLock;
    }

    // check for lock mismatch
    if(conditionLock != CVLock){
        printf("ERROR: Lock Mismatch for Wait on CV %s", name);
    }

    // let this thread wait
    queue->Append((void*) currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();

    (void) interrupt->SetLevel(oldLevel);
#endif
}

void Condition::Signal(Lock* conditionLock) {
#ifdef CHANGED
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    // if no one is waiting, do nothing
    if(queue->IsEmpty()) {
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    // someone is waiting...
    if(CVLock != conditionLock){
        printf("ERROR: Lock Mismatch for condition %s", name);
        return;
    } else {
        Thread *thread = (Thread *)queue->Remove();
        scheduler->ReadyToRun(thread);
    }

    // did that clear the queue?
    if(queue->IsEmpty()){
        CVLock = NULL;
    }

    (void) interrupt->SetLevel(oldLevel);
#endif
}
void Condition::Broadcast(Lock* conditionLock) {
#ifdef CHANGED
 IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if(CVLock == conditionLock){
    while(!queue->IsEmpty()){
      this->Signal(conditionLock);
    };
    (void) interrupt->SetLevel(oldLevel);
  }
#endif
}
  
