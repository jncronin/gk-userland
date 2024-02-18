#include "syscalls.h"
#include <errno.h>

int lstat(const char *path, struct stat *buf)
{
    int ret, _errno;
    struct __syscall_stat_params p;
    p.pathname = path;
    p.st = buf;
    __syscall(__syscall_stat, &ret, &p, _errno);
    if(ret)
    {
        errno = _errno;
    }
    return ret;
}
