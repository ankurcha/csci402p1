#include "syscall.h"

main(){
	int cvId = CreateCondition("New CV");
	DestroyCondition(cvId);
	if(cvId >= 0){
		DestroyCondition(cvId);
}
	
}