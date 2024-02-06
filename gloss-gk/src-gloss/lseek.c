#include <syscalls.h>

int _lseek(int file, int offset, int whence)
{
	int ret;
	struct __syscall_lseek_params p;
	p.file = file;
	p.offset = offset;
	p.whence = whence;
	__syscall(__syscall_lseek, &ret, &p, NULL);
	return ret;
}
