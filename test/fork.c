#include "syscall.h"

void func1( int arg ){
    int i;
    Write("Fork 1 \n",7,1);
    for(i=0;i<1000;i++){
        Yield();
    }
    Write("Fork 1 exit\n",20,1);
    Exit(0);
}

void main(){
    Fork(func1);
    Exit(0);
}
