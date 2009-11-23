#include "syscall.h"

char* itoa(int a) {
    static char str[50];
    int i = 49;
    do {
        str[i] = '0' + a % 10;
    } while ((a = a / 10) && i >= 0);

    return str;
}

int main() {
    int w = CreateLock("lockTest");
    Write(itoa(0), 1, 1);
    if (w >= 0) {
        Write("Pass2\n", 6, 1);
    } else {
        Write("Fail3\n", 6, 1);
    }
}
