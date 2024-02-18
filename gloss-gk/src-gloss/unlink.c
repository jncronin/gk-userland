#include <syscalls.h>
#include <errno.h>

int _unlink(char *name)
{
	int ret, _errno;
	__syscall(__syscall_unlink, &ret, name, &_errno);
	if(ret)
		errno = _errno;
	return ret;
}

