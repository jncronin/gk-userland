#include <syscalls.h>

int _stat(char *file, struct stat *st)
{
	int ret;
	__syscall(__syscall_stat, &ret, file, st);
	return ret;
}

