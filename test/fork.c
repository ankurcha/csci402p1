#include "syscall.h"
#include "print.c"
void func1(){
    print("fork");
  Exit(0);
}

void main(){
  Fork(func1);
  Exit(0);
}
