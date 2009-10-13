/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock;
int cv,i;

void a(void);
void main() {
	
	Write("Attempting Create Lock!\n",24,ConsoleOutput);
	lock = CreateLock();
	
	Write("Attempting CV Equivalent!\n",26,ConsoleOutput);
	cv = CreateCondition();
	DestroyCondition(cv);
	
	Write("Acquire/Release Valid \n",23,ConsoleOutput);
	Acquire(0);
	for(i=0;i<100;i++)
		Yield();
	Write("1st thread acquired lock\n",25,ConsoleOutput);
	Release(0);
	Write("1st thread releasing lock\n",26,ConsoleOutput);
	
	Write("Destroy Valid Lock\n",19,ConsoleOutput);
	for(i=0;i<100;i++)
		Yield();
	Exit(lock);
}
