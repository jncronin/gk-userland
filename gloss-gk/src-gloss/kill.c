#include <syscalls.h>
#include <errno.h>

int _kill(int pid, int sig)
{
	int ret, _errno;
	struct __syscall_kill_params p;
	p.pid = pid;
	p.sig = sig;
	__syscall(__syscall_kill, &ret, &p, &_errno);
	if(ret)
		errno = _errno;
	return ret;
}
