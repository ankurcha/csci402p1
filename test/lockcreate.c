#include "syscall.h"
#include "print.c"

main() {

int w = CreateLock("lock_test");
Acquire(w);
Release(w);
if(w>=0){
	print(" Testing lock creation...Pass\n");
 }
else {
	print(" Testing lock creation...Fail\n");
}


       }
