#include <syscalls.h>
#include <errno.h>

int _fork()
{
	int ret, _errno;
	__syscall(__syscall_fork, &ret, NULL, &_errno);
	if(ret == -1)
		errno = _errno;
	return ret;
}
