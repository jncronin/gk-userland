#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include "syscalls.h"
#include "deferred.h"
#include <errno.h>

extern "C" int ioctl(int fd, unsigned long op, ...)
{
    __syscall_ioctl_params p;
    p.fd = fd;
    p.nr = op;

    va_list args;
    va_start(args, op);
    p.ptr = va_arg(args, void *);
    p.len = 0;
    va_end(args);

    int ret;
    __syscall(__syscall_ioctl, (void *)&ret, (void *)&p, (void *)&errno);

    return ret;
}
