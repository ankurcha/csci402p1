#include "syscall.h"
#include "print.c"

int main(){
    print("Spawning 2 matmult processes\n");
    Exec("../test/matmult");
    Yield();
    Exec("../test/matmult");
}
