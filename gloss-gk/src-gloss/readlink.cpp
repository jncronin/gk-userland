#include <unistd.h>
#include <errno.h>
#include <string.h>

extern "C" ssize_t readlink(const char *pathname, char *buf, size_t bufsize)
{
    if(!bufsize)
    {
        errno = EINVAL;
        return -1;
    }

    if(!buf)
    {
        errno = EFAULT;
        return -1;
    }

    if(!pathname)
    {
        errno = EINVAL;
        return -1;
    }

    // stub - just copy source to dest
    strncpy(buf, pathname, bufsize);
    buf[bufsize - 1] = 0;
    return strlen(buf) + 1;
}
