#include <syscalls.h>

int _unlink(char *name)
{
	int ret;
	__syscall(__syscall_unlink, &ret, name, NULL);
	return ret;
}

