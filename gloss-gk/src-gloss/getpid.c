#include <syscalls.h>
#include <errno.h>

int _getpid()
{
	int ret;
	__syscall(__syscall_getpid, &ret, NULL, NULL);
	return ret;
}

int getppid()
{
	int pid = _getpid();
	int ret, _errno;

	__syscall(__syscall_getppid, &ret, (void *)(intptr_t)pid, &_errno);
	if(ret < 0)
	{
		errno = _errno;
	}
	return ret;
}

int setsid()
{
	return _getpid();
}
