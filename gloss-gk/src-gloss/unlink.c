#include <gk_syscalls.h>

int _unlink(char *name)
{
	return __gk_syscalls->_unlink(name);
}

