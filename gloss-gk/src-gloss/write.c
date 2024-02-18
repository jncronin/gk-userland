#include <syscalls.h>
#include <errno.h>

int _write(int file, char *buf, int nbytes)
{
	struct __syscall_read_params p;
	p.file = file;
	p.ptr = buf;
	p.len = nbytes;
	int ret, _errno;
	__syscall(__syscall_write, &ret, &p, &_errno);
	if(ret == -1)
		errno = _errno;
	return ret;
}
