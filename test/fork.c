#include "syscall.h"
#include "print.c"

void func1(){
    print("func1 forked and running\n");
    Exit(0);
}

void main(){
    Fork(func1);
    Yield();
    Exit(0);
}
