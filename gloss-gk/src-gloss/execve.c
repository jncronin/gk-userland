#include <syscalls.h>
#include <errno.h>

int _execve(char *name,
		char **argv,
		char **env)
{
	struct __syscall_execve_params p;
	p.name = name;
	p.argv = argv;
	p.env = env;
	int ret, _errno;
	__syscall(__syscall_execve, &ret, &p, &_errno);
	if(ret)
	{
		errno = _errno;
	}
	return ret;
}
