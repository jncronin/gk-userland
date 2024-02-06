#include <syscalls.h>

int _link(char *old, char *new)
{
	int ret;
	__syscall(__syscall_link, &ret, old, new);
	return ret;
}
