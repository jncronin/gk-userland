#include "syscalls.h"
#include "deferred.h"

#include <sys/stat.h>
#include <unistd.h>

extern "C" int mkfifo(const char *pathname, mode_t mode)
{
    __syscall_mkdir_params p { pathname, mode };
    return deferred_call(__syscall_mkfifo, &p);
}

extern "C" int mkdir(const char *pathname, mode_t mode)
{
    __syscall_mkdir_params p { pathname, mode };
    return deferred_call(__syscall_mkdir, &p);
}

extern "C" int rmdir(const char *pathname)
{
    return deferred_call(__syscall_rmdir, (void *)pathname);
}

extern "C" int chmod(const char *pathname, mode_t mode)
{
    __syscall_mkdir_params p { pathname, mode };
    return deferred_call(__syscall_chmod, &p);
}
