#include <gk_syscalls.h>

int _link(char *old, char *new)
{
	return __gk_syscalls->_link(old, new);
}
