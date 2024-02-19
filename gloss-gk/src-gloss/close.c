#include <syscalls.h>
#include <errno.h>

int _close(int file)
{
	int ret, _errno;
	__syscall(__syscall_close, &ret, (void *)file, &_errno);
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
