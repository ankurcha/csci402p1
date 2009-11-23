#include "syscall.h"
#include "print.c"

int main() {
    print("Spawning 2 multiForkmatmult processes\n");
    Exec("../test/multiForkmatmult");
    print("Spawned 1 multiForkmatmult processes\n");
    Yield();
    Exec("../test/multiForkmatmult");
    Yield();
    print("Spawned 2 multiForkmatmult processes\n");
    Exit(2);
}
