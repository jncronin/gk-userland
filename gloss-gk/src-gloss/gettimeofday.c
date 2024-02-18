#include <syscalls.h>
#include <sys/time.h>
#include <errno.h>

int _gettimeofday(struct timeval *tv, void *tz)
{
    int ret, _errno;
    struct __syscall_gettimeofday_params p;
    p.tv = tv;
    p.tz = tz;
    __syscall(__syscall_gettimeofday, &ret, &p, &_errno);
    if(ret)
        errno = _errno;
    return ret;
}

