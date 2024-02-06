#include <syscalls.h>

int _close(int file)
{
	int ret;
	__syscall(__syscall_close, &ret, (void *)file, NULL);
	return ret;
}
