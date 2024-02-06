#include <syscalls.h>

int _execve(char *name,
		char **argv,
		char **env)
{
	struct __syscall_execve_params p;
	p.name = name;
	p.argv = argv;
	p.env = env;
	int ret;
	__syscall(__syscall_execve, &ret, &p, NULL);
	return ret;
}
