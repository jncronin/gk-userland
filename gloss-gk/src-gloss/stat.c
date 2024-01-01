#include <gk_syscalls.h>

int _stat(char *file, struct stat *st)
{
	return __gk_syscalls->_stat(file, st);
}

