#include <syscalls.h>
#include <errno.h>

int _isatty(int file)
{
	int ret, _errno;
	__syscall(__syscall_isatty, &ret, (void *)file, &_errno);
	if(ret == 0)
		errno = _errno;
	return ret;
}
