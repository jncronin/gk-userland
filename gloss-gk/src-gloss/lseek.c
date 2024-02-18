#include <syscalls.h>
#include <errno.h>

int _lseek(int file, int offset, int whence)
{
	int ret, _errno;
	struct __syscall_lseek_params p;
	p.file = file;
	p.offset = offset;
	p.whence = whence;
	__syscall(__syscall_lseek, &ret, &p, &_errno);
	if(ret == (off_t)-1)
		errno = _errno;
	return ret;
}
