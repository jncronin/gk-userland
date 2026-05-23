#include "syscalls.h"
#include "deferred.h"
#include "gk.h"

int GK_DMABufAlloc(size_t len)
{
	int ret;
	int _errno;
	__syscall(__syscall_dmabuf_alloc, &ret, (void*)(intptr_t)len, &_errno);
	if(ret == -1)
		errno = _errno;
	return ret;
}
