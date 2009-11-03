#include "syscall.h"
#include "print.c"

int main(){
    print("Spawning 2 multiForkmatmult processes\n");
    Exec("../test/multiForkmatmult");
    Exec("../test/multiForkmatmult");
}
