#include <syscalls.h>
#include <errno.h>

int _close(int file)
{
	int ret, _errno;
	__syscall(__syscall_close, &ret, (void *)file, &_errno);
	if(ret)
		errno = _errno;
	return ret;
}
