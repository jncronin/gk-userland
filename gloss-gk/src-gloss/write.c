#include <syscalls.h>

int _write(int file, char *buf, int nbytes)
{
	struct __syscall_read_params p;
	p.file = file;
	p.ptr = buf;
	p.len = nbytes;
	int ret;
	__syscall(__syscall_write, &ret, &p, NULL);
	return ret;
}
