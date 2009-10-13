#include "syscall.h"
#include "print.c"
void func1( ){
    print("Forked func1, yielding for 1 cycle");
  Yield();
  print("func1: Exiting with status code -1");
  Exit(-1);
}

void main(){
  Fork(func1);
  Exit(0);
}
