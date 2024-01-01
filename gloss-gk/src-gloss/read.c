#include <gk_syscalls.h>

int _read(int file, char *ptr, int len)
{
	return __gk_syscalls->_read(file, ptr, len);
}
