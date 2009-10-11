#include "syscall.h"

#ifndef PRINT_C
#define PRINT_C

void print(char* str) {
    int i = 0;
    while(str[i] != 0) {
        i++;
    }
    i++;
    Write(str, i, ConsoleOutput);
    return;
}

#endif /* PRINT_C */
