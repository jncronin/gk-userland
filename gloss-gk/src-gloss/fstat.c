#include <gk_syscalls.h>
#include <sys/stat.h>

int _fstat(int file, struct stat *st)
{
	return __gk_syscalls->_fstat(file, st);
}
