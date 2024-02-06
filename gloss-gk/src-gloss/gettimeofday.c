#include <syscalls.h>
#include <sys/time.h>

int _gettimeofday(struct timeval *tv, void *tz)
{
    int ret;
    __syscall(__syscall_gettimeofday, &ret, tv, tz);
    return ret;
}
