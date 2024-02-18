#include <syscalls.h>
#include <errno.h>

int _stat(char *file, struct stat *st)
{
	int ret, _errno;
	struct __syscall_stat_params p;
	p.pathname = file;
	p.st = st;
	__syscall(__syscall_stat, &ret, &p, &_errno);
	if(ret)
		errno = _errno;
	return ret;
}

