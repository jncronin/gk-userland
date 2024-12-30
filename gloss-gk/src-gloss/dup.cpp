#include <unistd.h>
#include <errno.h>
#include "syscalls.h"
#include "deferred.h"

extern "C" int dup(int oldfd)
{
    __syscall_dup_params p { .fd1 = oldfd, .fd2 = -1 };
    return deferred_call(__syscall_dup2, &p);
}

extern "C" int dup2(int oldfd, int newfd)
{
    __syscall_dup_params p { .fd1 = oldfd, .fd2 = newfd };
    return deferred_call(__syscall_dup2, &p);
}
