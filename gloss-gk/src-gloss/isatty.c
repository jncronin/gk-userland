#include <syscalls.h>

int _isatty(int file)
{
	int ret;
	__syscall(__syscall_isatty, &ret, (void *)file, NULL);
	return ret;
}
