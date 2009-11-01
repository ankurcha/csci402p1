#include "syscall.h"
#include "print.c"

void helloWorld(){
    print("hello World\n");
    Exit(7);
}

int main(){

    print("Forking matmult\n");
    Fork(helloWorld);
    Yield();
    print("Exiting Main\n");
    Exit(0);
}
