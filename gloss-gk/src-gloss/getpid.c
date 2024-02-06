#include <syscalls.h>

int _getpid()
{
	int ret;
	__syscall(__syscall_getpid, &ret, NULL, NULL);
	return ret;
}
