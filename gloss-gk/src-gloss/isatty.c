#include <gk_syscalls.h>

int _isatty(int file)
{
	return __gk_syscalls->_isatty(file);
}
