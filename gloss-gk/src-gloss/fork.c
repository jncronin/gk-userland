#include <syscalls.h>

int _fork()
{
	int ret;
	__syscall(__syscall_fork, &ret, NULL, NULL);
	return ret;
}
