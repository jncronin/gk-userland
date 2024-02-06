#include <syscalls.h>

int _kill(int pid, int sig)
{
	int ret;
	__syscall(__syscall_kill, &ret, (void *)pid, (void *)sig);
	return ret;
}
