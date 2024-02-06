#include <syscalls.h>

int _fstat(int file, struct stat *st)
{
	int ret;
	__syscall(__syscall_fstat, &ret, (void *)file, st);
	return file;
}
