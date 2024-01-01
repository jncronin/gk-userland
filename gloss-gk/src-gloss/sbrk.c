#include <gk_syscalls.h>

void *_sbrk(int nbytes)
{
	return __gk_syscalls->_sbrk(nbytes);
}
