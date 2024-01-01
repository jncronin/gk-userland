#include <gk_syscalls.h>

int _open(const char *name, int flags, int mode)
{
	return __gk_syscalls->_open(name, flags, mode);
}
