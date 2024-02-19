#include <syscalls.h>
#include <errno.h>

int _open(const char *name, int flags, int mode)
{
	int ret, _errno;
	struct __syscall_open_params p;
	p.name = name;
	p.flags = flags;
	p.mode = mode;
	__syscall(__syscall_open, &ret, &p, &_errno);
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
