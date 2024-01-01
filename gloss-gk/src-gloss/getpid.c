#include <gk_syscalls.h>

int _getpid()
{
	return __gk_syscalls->_getpid();
}
