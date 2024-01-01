#include <gk_syscalls.h>
#include <sys/stat.h>

int _stat(char *file, struct stat *st)
{
	return __gk_syscalls->_stat(file, st);
}

