#include "syscalls.h"
#include "deferred.h"
#include <utime.h>

extern "C" int utime(const char *filename, const struct utimbuf *times)
{
    // not yet implemented
    return 0;
}
