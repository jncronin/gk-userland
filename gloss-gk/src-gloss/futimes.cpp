#include <sys/stat.h>
#include <errno.h>

extern "C" int futimens(int fd, const struct timespec times[2])
{
    errno = ENOTSUP;
    return -1;
}
