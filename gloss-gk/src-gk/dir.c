#include "syscalls.h"
#include "errno.h"
#include <sys/types.h>
#include <dirent.h>

int fchdir(int fd)
{
    int ret, _errno;
    __syscall(__syscall_fchdir, &ret, (void *)fd, &_errno);
    if(_errno)
    {
        errno = _errno;
    }
    return ret;
}

int chdir(const char *path)
{
    int ret, _errno;
    __syscall(__syscall_chdir, &ret, (void *)path, &_errno);
    if(_errno)
    {
        errno = _errno;
    }
    return ret;
}

char *getcwd(char *buf, size_t size)
{
    int _errno;
    __syscall(__syscall_getcwd, buf, (void*)size, &_errno);
    if(_errno)
    {
        errno = _errno;
        return NULL;
    }
    return buf;
}
