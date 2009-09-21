// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "test_code.cc"
#include "init.cc"

//#include "syncListImplementation.cc"
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
      	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);

}

//----------------------------------------------------------------------
// Problem2
//      Test the hospital management simulation
//----------------------------------------------------------------------

void Problem2(int choice = -1)
{
    while (true) {
        if(choice == -1) {
            printf("\nTESTING HOSPITAL SIMULATION\n");
            cerr << "\nSelect from the options below"<<endl
            <<"    0. Run simulation normally."<<endl
            <<"    1. Run Test 1 (Patient only gets in when doorboy asks)\n"
            <<"    2. Run Test 2 (If doorboy is on break, no patient gets in)\n"
            <<"    3. Run Test 3 (One patient with doctor at a time)\n"
            <<"    4. Run Test 4 (Patients always choose shortest line)\n"
            <<"    5. Run Test 5 (If no cashier/clerk/receptionist patient waits)\n"
            <<"    6. Run Test 6 (doctor goes on break at random intervals)\n"
            <<"    7. Run Test 7 (when the doctor is on break, no patient gets in)\n"
            <<"    8. Run Test 8 (doorboy never goes on break when doctor is on break)\n"
            <<"    9. Run Test 9 (doorboy/cashier/clerk get signaled by manager when patients waiting)\n"
            <<"    10. Run Test 10 (sales and fees not affected by race conditions)\n"
            <<"    11. Run Test 11 (doorboy/clerk/receptionist/cashier go on break when line is empty)\n";
            cin>>choice;
        }
        switch ( choice ) {
            case 0:
                HospINIT();
                return;
                break;
            case 1:
                test1();
                return;
                break;
            case 3:
                HospINIT(3);
                return;
                break;
            case 4:
                test4();
                return;
                break;
            case 6:
                HospINIT(6);
                return;
                break;
            case 8:
                HospINIT(8);
                return;
                break;
            case 10:
                HospINIT(10);
                return;
                break;
            default:
                choice = -1;
                break;
        }
    }
}


