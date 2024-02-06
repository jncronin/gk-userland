#include <syscalls.h>

void *_sbrk(int nbytes)
{
	void *ret;
	__syscall(__syscall_sbrk, &ret, (void*)nbytes, NULL);
	return ret;
}
