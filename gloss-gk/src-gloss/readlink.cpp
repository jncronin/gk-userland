#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "deferred.h"
#include "syscalls.h"

extern "C" ssize_t readlink(const char *pathname, char *buf, size_t bufsize)
{
    if(!bufsize)
    {
        errno = EINVAL;
        return -1;
    }

    if(!buf)
    {
        errno = EFAULT;
        return -1;
    }

    if(!pathname)
    {
        errno = EINVAL;
        return -1;
    }

    __syscall_readlink_params p
    {
        .pathname = pathname,
        .buf = buf,
        .bufsize = bufsize
    };
    return deferred_call(__syscall_readlink, &p);
}
