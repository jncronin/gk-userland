#include <syscalls.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

int _fstat(int file, struct stat *st)
{
	int ret, _errno;
	struct __syscall_fstat_params p;
	p.buf = st;
	p.fd = file;
	__syscall(__syscall_fstat, &ret, &p, &_errno);
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
	return file;
}

int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags)
{
	// TODO
	return stat(pathname, buf);
}

int fstatvfs(int fd, struct statvfs *buf)
{
	if(fd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	buf->f_flag |= MNT_LOCAL;
	return 0;
}
