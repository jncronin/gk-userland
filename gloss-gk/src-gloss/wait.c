#include <syscalls.h>
#include <errno.h>

int _wait(int *status)
{
	int ret, _errno;
	__syscall(__syscall_wait, &ret, status, &_errno);
	if(ret == -1)
		errno = _errno;
	return ret;
}

