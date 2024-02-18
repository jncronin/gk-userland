#include <syscalls.h>
#include <errno.h>

void *_sbrk(int nbytes)
{
	void *ret;
	int _errno;
	__syscall(__syscall_sbrk, &ret, (void*)nbytes, &_errno);
	if(ret == (void*)-1)
		errno = _errno;
	return ret;
}
