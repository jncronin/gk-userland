#include <gk_syscalls.h>

void _exit(int rc)
{
	__gk_syscalls->_exit(rc);
}
