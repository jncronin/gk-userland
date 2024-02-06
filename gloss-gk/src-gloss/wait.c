#include <syscalls.h>

int _wait(int *status)
{
	int ret;
	__syscall(__syscall_wait, &ret, status, NULL);
	return ret;
}

