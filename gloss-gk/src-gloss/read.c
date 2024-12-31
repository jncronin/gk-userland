#include <syscalls.h>
#include <errno.h>

int _read(int file, char *ptr, int len)
{
	while(1)
	{
		struct __syscall_read_params p;
		p.file = file;
		p.ptr = ptr;
		p.len = len;
		int ret, _errno;
		__syscall(__syscall_read, &ret, &p, &_errno);
		if(ret == -1)
		{
			errno = _errno;
			return ret;
		}
		else if(ret == -2)	// Deferred return
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
		else if(ret >= 0)
			return ret;
		// else ret == -3 -> try again
	}
}
