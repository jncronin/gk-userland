#include <syscalls.h>
#include <errno.h>
#include <sys/wait.h>
#include "deferred.h"

extern "C" pid_t waitpid(pid_t pid, int *status, int options)
{
	if(pid < -1)
	{
		errno = EINVAL;
		return -1;
	}

	__syscall_waitpid_params p { .pid = pid, .stat_loc = status, .options = options };
	auto ret = deferred_call_with_retry(__syscall_waitpid, &p);
	if(ret < -1)
		return -1;
	return ret;
}

extern "C" int _wait(int *status)
{
	return waitpid(-1, status, 0);
}

extern "C" int wait4(pid_t pid, int *wstatus, int options, rusage *rusage)
{
	return waitpid(pid, wstatus, options);
}
