#include "syscall.h"
#include "print.h"
void func1( ){
  Write("func1: Forked\n",14,1);
  Exit(0);
}

void main(){
  Fork(func1);
}
