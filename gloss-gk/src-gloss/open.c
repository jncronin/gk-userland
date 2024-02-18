#include <syscalls.h>
#include <errno.h>

int _open(const char *name, int flags, int mode)
{
	int ret, _errno;
	struct __syscall_open_params p;
	p.name = name;
	p.flags = flags;
	p.mode = mode;
	__syscall(__syscall_open, &ret, &p, &_errno);
	if(ret == -1)
		errno = _errno;
	return ret;
}
