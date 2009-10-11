#include "syscall.h"
#include "print.c"

int LockId2;/*Integer type Lock Id*/
int ConditionId2; /* Integer type condition Id*/
void p2test1();
void p2test1_thread2();
void p2test2();
void p2test2_thread2();
void p2test2_thread3();
void p2test3();
void p2test3_thread2();
void p2test3_thread3();
void p2test3_thread4();
int main()
{
    print("test1 start\n");
    print("test2 start\n");
    /*print("test3 start\n");*/
    p2test1();
    p2test1_thread2();
    
    p2test2();
    p2test2_thread2();
    p2test2_thread3();
    
    p2test3();
    p2test3_thread2();
    p2test3_thread3();
    p2test3_thread4();
    
    
        /*Acquire(LockId2);                                                        */
    /* Calling of Acquire function*/
    
        /*Release(LockId2);                                                        */
    /* Calling of Release function*/
    
        /*DestroyLock(LockId2);                                                    */
    /* Calling of Destroy Lock Function*/
    
        /*LockId2= CreateLock("");                                                   */
    /* Called Create Lock Function and returning the integer value*/
    
        /*Wait(ConditionId2, LockId2);                                          */
    /* Wait function called*/
    
    
        /*Signal(ConditionId2, LockId2);                                         */
    /* Signal function is called*/
    
        /*Broadcast(ConditionId2, LockId2);                                      */
    /* Broadcast function is called i.e waking up all sleeping threads */
    
    
        /*DestroyCondition(ConditionId2);                                        */
    /* Destroy the created condition */
    
    
        /*ConditionId2= CreateCondition("");                                         */
    /* Create Condition */
    
    Yield();
}

void p2test1()
{
	LockId2= CreateLock("t1_l1");                                             
    /* Lock is Created */
	Fork(p2test1_thread2);                                               
    /* Fork to create second thread */
	print("thread 1 acquiring lock\n" );              
    /* printout for acquiring a lock */
	Acquire(LockId2);                                                   
    /* Calling a Acquire function */
	print("thread 1 lock acquired\n" );               
    /* Printout that lock is Acquired */
	print("thread 1 lock released\n" );               
    /* Printout that lock is released*/
	Release(LockId2);                                                   
    /* Calling a Release function*/
}

void p2test1_thread2()
{
	Acquire(LockId2);                                                   
    /* Calling a acquire function*/
	print("thread 2 lock acquired\n" );               
    /* Printout for acquiring a lock*/
	print("thread 2 lock released\n" );               
    /* Printout for releasing a lock*/
	Release(LockId2);
}

void p2test2()
{
	Fork(p2test2_thread2);                                                
    /* Fork to create second thread*/
	Fork(p2test2_thread3);                                               
    /* Fork to create third thread */
	print("thread 1 acquiring lock\n" );              
    /* Printout for acquiring a lock*/
	Acquire(LockId2);                                                  
    /* Calling the Acquire function */
	print("thread 1 lock acquired\n" );               
    /* Printout that lock is acquired*/
	Release(LockId2);                                                  
    /* Calling the release function */
	print("thread 1 lock released\n" );
}

void p2test2_thread2()
{
	Acquire(LockId2);                                                  
    /* Calling a Acquire Function*/                        
	print("thread 2 acquire lock\n" );                  
    /* Printout that lock is acquired*/
	DestroyLock(LockId2);                                                
    /* Destroying a lock */
	print("thread 2 destroy lock\n" );                  
    /* Printout that lock is destroyed */
}

void p2test2_thread3()
{	
	LockId2= CreateLock("t2_l2");                                                 
    /* Lock is created*/
	print("thread 3 acquiring lock\n" );                  
    /* Printout for acquiring a lock*/
	Release(LockId2);                                                      
    /* Lock is released */
	print("thread 3 releasing lock\n" );                  
    /* Printout that lock is released*/
}	

void p2test3()
{
	ConditionId2= CreateCondition("t3_c1");                                                                   
    /* Condition variable is created */
	print("thread 1  creating condition variable\n" );                                
    /* Printout for creating a condition variable*/
	LockId2= CreateLock("t1_l1");                                                                             
    /* Lock is created */
	print("thread 1 acquiring lock\n" );                                              
    /* Printout for acquiring a lock*/
	Fork(p2test3_thread2);                                                                             
    /* Fork to create second thread*/
	Fork(p2test3_thread3);                                                                             
    /* Fork to create third thread */
	Fork(p2test3_thread4);                                                                             
    /* Fork to create fourth thread */
	Wait(ConditionId2, LockId2);                                                                       
    /* Wait signal is called*/
	print("thread 1 waiting on a condition\n" );                                      
    /* Printout for a thread on wait*/
	Signal(ConditionId2, LockId2);                                                                     
    /* Signal is called*/
	print("thread 1 signalling a thread to wake up\n");                              
    /* Printout for a signal*/
	Broadcast(ConditionId2, LockId2);                                                                  
    /* Broadcast function is called*/
	print("thread 1 broadcasting \n" );                                               
    /* Printout that on broadcasting*/ 
}

void p2test3_thread2()
{
	LockId2= CreateLock("t1_l2");                                                          
    /* Lock is created */
	print("thread 2 creating a lock\n" );                
	Wait(ConditionId2, LockId2);                                                    
    /*Wait function is called*/                                  
	print("thread 2 waiting on a condition\n" );
	Signal(ConditionId2, LockId2);                                                  
    /*Signal fuction is called*/
	print("signalling thread 2\n" );
	Broadcast(ConditionId2, LockId2);                                               
    /* Broadcast fuction is called*/
	print("thread 2 broadcasting\n");
	DestroyCondition(ConditionId2);                                                 
    /* Destroying a condition variable */
}

void p2test3_thread3()
{
	ConditionId2= CreateCondition("t1_c2");                                       
    /* Condition variable is created */
	print("thread 3 creating a condition variable\n"); 
	Signal(ConditionId2, LockId2);                                         
    /*Signal function is called*/
	print("thread 3 signalling a thread to wake up\n");
	Broadcast(ConditionId2, LockId2);                                      
    /*Broadcast function is called*/
	print("thread 3 broadcasting \n");
}

void p2test3_thread4()
{
	
	Wait(ConditionId2, LockId2);                          
	print("thread 4 waiting on a condition \n");
	Signal(ConditionId2, LockId2);                       
	print("signalling thread 4\n");
	Broadcast(ConditionId2, LockId2);
	print("thread 4 broadcasting\n");
	DestroyCondition(ConditionId2);                           
    /* Destroying a condition variable */
}

