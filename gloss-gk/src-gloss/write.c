#include <syscalls.h>
#include <errno.h>

int _write(int file, char *buf, int nbytes)
{
	struct __syscall_read_params p;
	p.file = file;
	p.ptr = buf;
	p.len = nbytes;
	int ret, _errno;
	__syscall(__syscall_write, &ret, &p, &_errno);
	if(ret == -1)
		errno = _errno;
	if(ret == -2)	// Deferred return
	{
		struct WaitSimpleSignal_params wssp;
		while(1)
		{
			uint32_t wss;
			__syscall(WaitSimpleSignal, &wss, &wssp, NULL);
			if(wss)
				break;
		}
		if(wssp.ival1 == -1)
			errno = wssp.ival2;
		return wssp.ival1;
	}
	return ret;
}
