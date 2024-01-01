#include <gk_syscalls.h>

int _execve(char *name,
		char **argv,
		char **env)
{
	return __gk_syscalls->_execve(name, argv, env);
}
