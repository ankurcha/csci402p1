#include "syscall.h"
#include "print.c"

int main(){
    print("Spawning 3 Hospital simulations\n");
    Exec("../test/init");
    Exec("../test/init");
    Exec("../test/init");
}

