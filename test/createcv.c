# include"syscall.h"


main() {
    int w = CreateCondition("CreateCV");
    if(w>=0){
    	Write("Testing CV creation...Pass\n" , 27, ConsoleOutput );
    }else {
        Write("Testing CV creation...Fail\n" , 27, ConsoleOutput );
    }
    DestroyCondition(w);
}
