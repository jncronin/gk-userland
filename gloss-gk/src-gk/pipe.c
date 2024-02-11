#include <unistd.h>
#include <errno.h>
#include "syscalls.h"

int pipe(int pipefd[2])
{
    int ret, _errno;
    __syscall(__syscall_pipe, &ret, pipefd, &_errno);
    if(_errno)
    {
        errno = _errno;
    }
    return ret;
}
