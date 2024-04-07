#include "syscalls.h"
#include "deferred.h"

#include <sys/statvfs.h>
#include <errno.h>

extern "C" int statvfs(const char *path,
    struct statvfs *buf)
{
    // not yet implemented
    errno = ENOSYS;
    return -1;
}
