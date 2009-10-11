#include "syscall.h"

void print(char* str) {
    int i = 0;
    while(str[i] != 0) {
        i++;
    }
    i++;
    Write(str, i, ConsoleOutput);
    return;
}

