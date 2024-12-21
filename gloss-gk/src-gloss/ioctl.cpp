#include <stdio.h>
#include <errno.h>

extern "C" int ioctl(int fd, unsigned long op, ...)
{
    fprintf(stderr, "ioctl(%u, %x) unsupported\n", fd, op);
    errno = EINVAL;
    return -1;
}
