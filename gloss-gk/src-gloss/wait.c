#include <gk_syscalls.h>

int _wait(int *status)
{
	return __gk_syscalls->_wait(status);
}

