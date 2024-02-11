#include "syscalls.h"
#include <errno.h>

int dup(int oldfd)
{
    int ret, _errno;
    __syscall(__syscall_dup, &ret, (void *)oldfd, &_errno);
    if(_errno != 0)
    {
        errno = _errno;
    }
    return ret;
}
