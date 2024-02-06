#include <syscalls.h>

void _exit(int rc)
{
	__syscall(__syscall_exit, (void *)rc, NULL, NULL);
}
