#include <unistd.h>
#include <errno.h>

extern "C" int symlink(const char *target, const char *linkpath)
{
    errno = EPERM;
    return -1;
}
