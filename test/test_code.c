#include "syscall.h"
#include "print.c"

LockId lockId;/*Integer type Lock Id*/
CVId conditionId; /* Integer type condition Id*/
void test1();
void test1_t();
void test2();
void test2_thread2();
void test2_thread3();
void test3();
void test3_thread2();
void test3_thread3();
void test3_thread4();
int main() {
    print("Starting Test1\n");

    print("Starting Test2\n");
    print("test3 start\n");
    test1();
    test2();
    test3();
    Yield();
    Exit(0);
}

void test1() {
    lockId = CreateLock("");
    /* Lock is Created */
    Fork(test1_t);
    /* Fork to create second thread */
    Acquire(lockId);
    /* Calling a Acquire function */
    print("Thread 1 lock acquired\n");
    /* Printout that lock is Acquired */
    Release(lockId);
    /* Calling a Release function*/
    print("Thread 1 lock released\n");

}

void test1_t() {
    Acquire(lockId);
    /* Calling a acquire function*/
    print("thread 2 lock acquired\n");
    /* Printout for releasing a lock*/
    Release(lockId);
    print("thread 2 lock released\n");
    Exit(11);
}

void test2() {
    Fork(test2_thread2);
    /* Fork to create second thread*/
    Fork(test2_thread3);
    /* Fork to create third thread */
    print("thread 1 acquiring lock\n");
    /* Printout for acquiring a lock*/
    Acquire(lockId);
    /* Calling the Acquire function */
    print("thread 1 lock acquired\n");
    /* Printout that lock is acquired*/
    Release(lockId);
    /* Calling the release function */
    print("thread 1 lock released\n");

}

void test2_thread2() {
    Acquire(lockId);
    /* Calling a Acquire Function*/
    print("thread 2 acquire lock\n");
    /* Printout that lock is acquired*/
    DestroyLock(lockId);
    /* Destroying a lock */
    print("thread 2 destroy lock\n");
    /* Printout that lock is destroyed */
    Exit(22);
}

void test2_thread3() {
    int i = 50;
    lockId = CreateLock("");
    /* Lock is created*/
    print("thread 3 acquiring lock\n");
    /* Printout for acquiring a lock*/
    for (i = 50; i > 0; i--) {
        Yield();
    }
    Release(lockId);
    /* Lock is released */
    print("thread 3 releasing lock\n");
    /* Printout that lock is released*/
    Exit(23);
}

void test3() {
    conditionId = CreateCondition("");
    /* Condition variable is created */
    print("thread 1  creating condition variable\n");
    /* Printout for creating a condition variable*/
    lockId = CreateLock("t3_l1");
    /* Lock is created */
    print("thread 1 acquiring lock\n");
    /* Printout for acquiring a lock*/
    Fork(test3_thread2);
    /* Fork to create second thread*/
    Fork(test3_thread3);
    /* Fork to create third thread */
    Fork(test3_thread4);
    /* Fork to create fourth thread */
    Wait(conditionId, lockId);
    /* Wait signal is called*/
    print("thread 1 waiting on a condition\n");
    /* Printout for a thread on wait*/
    Signal(conditionId, lockId);
    /* Signal is called*/
    print("thread 1 signalling a thread to wake up\n");
    /* Printout for a signal*/
    Broadcast(conditionId, lockId);
    /* Broadcast function is called*/
    print("thread 1 broadcasting \n");
    /* Printout that on broadcasting*/
}

void test3_thread2() {
    lockId = CreateLock("");
    /* Lock is created */
    print("thread 2 creating a lock\n");
    Wait(conditionId, lockId);
    /*Wait function is called*/
    print("thread 2 waiting on a condition\n");
    Signal(conditionId, lockId);
    /*Signal fuction is called*/
    print("signalling thread 2\n");
    Broadcast(conditionId, lockId);
    /* Broadcast fuction is called*/
    print("thread 2 broadcasting\n");
    DestroyCondition(conditionId);
    /* Destroying a condition variable */
    Exit(32);
}

void test3_thread3() {
    conditionId = CreateCondition("");
    /* Condition variable is created */
    print("thread 3 creating a condition variable\n");
    Signal(conditionId, lockId);
    /*Signal function is called*/
    print("thread 3 signalling a thread to wake up\n");
    Broadcast(conditionId, lockId);
    /*Broadcast function is called*/
    print("thread 3 broadcasting \n");
    Exit(33);
}

void test3_thread4() {

    Wait(conditionId, lockId);
    print("thread 4 waiting on a condition \n");
    Signal(conditionId, lockId);
    print("signalling thread 4\n");
    /*Broadcast(conditionId, lockId);*/
    print("thread 4 broadcasting\n");
    DestroyCondition(conditionId);
    /* Destroying a condition variable */
    Exit(34);
}

