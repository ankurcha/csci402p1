#include "syscall.h"
#include "print.c"

main() {
    int cvId = CreateCondition("New CV");
    DestroyCondition(cvId);
    print("Condition Variable Destroyed...trying once more, must fail\n");
    if (cvId >= 0) {
        DestroyCondition(cvId);
    }

}
