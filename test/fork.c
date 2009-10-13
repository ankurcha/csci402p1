/* This program tests the functionality of our Fork call, 
 *  by assuring that func1 is forked and run
 */

#include "syscall.h"
#include "print.c"

int lock, cv;

void func1(){
    print("func1 forked and running\n");

    Acquire(lock);
    print("func1 acquired lock\n");
    Signal(cv, lock);

    print("func1 Exiting with exit status 0\n");

    Release(lock);
    Yield();
    Exit(0);
}

void main(){
    lock = CreateLock("lock");
    cv = CreateCondition("condition");
    Acquire(lock);

    Fork(func1);

    print("main forked func1\n");
    Wait(cv, lock);
    print("main got signaled\n");
    Release(lock);
    print("main released lock\n");
    Yield();
    Exit(0);
}

