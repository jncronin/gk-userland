#include <syscalls.h>
#include <errno.h>

int _link(char *old, char *new)
{
	int ret, _errno;
	struct __syscall_link_params p;
	p.oldpath = old;
	p.newpath = new;
	__syscall(__syscall_link, &ret, &p, &_errno);
	if(ret)
		errno = _errno;
	return ret;
}
