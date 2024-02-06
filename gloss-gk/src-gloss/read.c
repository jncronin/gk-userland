#include <syscalls.h>

int _read(int file, char *ptr, int len)
{
	struct __syscall_read_params p;
	p.file = file;
	p.ptr = ptr;
	p.len = len;
	int ret;
	__syscall(__syscall_read, &ret, &p, NULL);
	return ret;
}
