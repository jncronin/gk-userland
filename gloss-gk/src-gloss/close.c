#include <gk_syscalls.h>

int _close(int file)
{
	return __gk_syscalls->_close(file);
}
