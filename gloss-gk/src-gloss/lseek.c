#include <gk_syscalls.h>

int _lseek(int file, int offset, int whence)
{
	return __gk_syscalls->_lseek(file, offset, whence);
}
