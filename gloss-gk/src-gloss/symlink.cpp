#include <unistd.h>
#include <errno.h>
#include "syscalls.h"
#include "deferred.h"

extern "C" int symlink(const char *target, const char *linkpath)
{
    if(!target || !linkpath)
    {
        errno = EINVAL;
        return -1;
    }

    __syscall_symlink_params p;
    p.target = target;
    p.path = linkpath;

    return deferred_call(__syscall_symlink, &p);
}
