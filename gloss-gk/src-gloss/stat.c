#include <syscalls.h>
#include <errno.h>

int _stat(char *file, struct stat *st)
{
	int ret, _errno;
	struct __syscall_stat_params p;
	p.pathname = file;
	p.st = st;
	__syscall(__syscall_stat, &ret, &p, &_errno);
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
		if(wssp.ival1)
			errno = wssp.ival2;
		return wssp.ival1;
	}
	return ret;
}

