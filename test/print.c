/* library to wrap our syscall Write
 *
 * CS 402 Fall 2009
 * Group 11
 */

#include "syscall.h"
#include "print.h"

void print(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        i++;
    }
    i++;
    Write(str, i, ConsoleOutput);
    return;
}

