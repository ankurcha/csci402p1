#include "syscall.h"


main() {

int w = CreateLock("lock_test");
Acquire(w);
Release(w);

if(w>=0){
	Write(" Testing lock creation...Pass\n" , 50, ConsoleOutput );
 }
else {
	Write(" Testing lock creation...Fail\n" , 50, ConsoleOutput );
}


       }