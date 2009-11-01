#include "syscall.h"
#include "print.c"

#define Dim 20

void helloWorld(){
    print("Hello World");
    Exit(13);
}

int main(){
    print("Forking matmult\n");
    Fork(helloWorld);
    print("Exiting Main\n");
    Exit(0);
}
