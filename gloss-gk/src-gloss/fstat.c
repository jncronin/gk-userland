#include <syscalls.h>
#include <errno.h>

int _fstat(int file, struct stat *st)
{
	int ret, _errno;
	struct __syscall_fstat_params p;
	p.buf = st;
	p.fd = file;
	__syscall(__syscall_fstat, &ret, &p, &_errno);
	if(ret)
		errno = _errno;
	return file;
}
