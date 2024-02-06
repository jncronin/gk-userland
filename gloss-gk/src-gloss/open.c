#include <syscalls.h>

int _open(const char *name, int flags, int mode)
{
	int ret;
	struct __syscall_open_params p;
	p.name = name;
	p.flags = flags;
	p.mode = mode;
	__syscall(__syscall_open, &ret, &p, NULL);
	return ret;
}
