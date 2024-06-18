#include <syscalls.h>
#include "deferred.h"
#include <errno.h>
#include <fcntl.h>

extern "C" int _stat(char *file, struct stat *st)
{
	int fd, ret;

	// open file then fstat it
	struct __syscall_open_params p { file, O_RDONLY, 0 };
	fd = deferred_call(__syscall_open, &p);

	if(fd < 0)
	{
		return -1;
	}

	struct __syscall_fstat_params p2 { fd, st };
	ret = deferred_call(__syscall_fstat, &p2);
	
	int saved_errno = 0;
	if(ret < 0)
	{
		saved_errno = errno;
	}

	deferred_call(__syscall_close1, (void *)fd);
	deferred_call(__syscall_close2, (void *)fd);
	if(ret < 0)
	{
		errno = saved_errno;
		return ret;
	}

	return ret;
}
