#include "syscall.h"

main() {
	int lId = CreateLock("New Lock");
	DestroyLock(lId);
	if (lId >= 0) {
		DestroyLock(lId);
	}
}
