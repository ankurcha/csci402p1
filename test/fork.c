#include "syscall.h"

void func1( ){
  Write("Fork 1\n",7,1);
  Yield();
  Write("Fork 1 exit\n",12,1);
  Exit(0);
}

void func2(){
  Write("Fork 2\n",7,1);
  Exec("../test/exectest");
  Exit(0);
}

void main(){
  Fork(func1);
  Fork(func2);
  Exit(0);
}
