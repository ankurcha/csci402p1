#include "syscall.h"
#include "print.c"
int main() {
	print("Spawning 3 Hospital simulations\n");
	/*Exec("/home/scf-10/ankurcha/code/test/init");
	 Exec("/home/scf-10/ankurcha/code/test/init");
	 Exec("/home/scf-10/ankurcha/code/test/init");
	 */
	Exec("../test/init");
	Exec("../test/init");
	Exec("../test/init");
}
