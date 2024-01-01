#include <gk_syscalls.h>

int _write(int file, char *buf, int nbytes)
{
	return __gk_syscalls->_write(file, buf, nbytes);
}

