#include <gk_syscalls.h>
#include <sys/time.h>

int _gettimeofday(struct timeval *tv, void *tz)
{
    return __gk_syscalls->_gettimeofday(tv, tz);
}
