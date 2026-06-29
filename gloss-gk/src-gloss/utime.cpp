#include "syscalls.h"
#include "deferred.h"
#include <utime.h>

extern "C" int utime(const char *filename, const struct utimbuf *times)
{
    // not yet implemented
    return 0;
}

extern "C" int utimes(const char *filename, const struct timeval times[2])
{
    // not yet implemented
    return 0;
}
