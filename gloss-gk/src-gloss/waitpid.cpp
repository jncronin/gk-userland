#include "syscalls.h"
#include <sys/types.h>
#include <errno.h>

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
    struct __syscall_waitpid_params p;
    p.pid = pid;
    p.stat_loc = stat_loc;
    p.options = options;
    pid_t ret;
    int _errno;
    __syscall(__syscall_waitpid, &ret, &p, &_errno);
    if(ret == (pid_t)-1)
    {
        errno = _errno;
    }
    return ret;
}
