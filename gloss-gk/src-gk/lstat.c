#include "syscalls.h"
#include <errno.h>

int lstat(const char *path, struct stat *buf)
{
    int ret;
    struct __syscall_stat_params p;
    p.path = path;
    p.buf = buf;
    __syscall(__syscall_stat, &ret, &p, NULL);
    if(p._errno)
    {
        errno = p._errno;
    }
    return ret;
}
