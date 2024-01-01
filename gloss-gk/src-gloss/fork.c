#include <gk_syscalls.h>

int _fork()
{
	return __gk_syscalls->_fork();
}
