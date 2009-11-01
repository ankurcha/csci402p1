#include "syscall.h"
#include "print.c"

#define Dim 20

int a[50];


void helloWorld(){
    print("HelloWorld\n");
    Exit(13);
}

int main(){

    print("Forking matmult\n");
    Fork(helloWorld);
    Yield();
    print("Exiting Main\n");
    Exit(0);
}
