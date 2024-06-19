#include <sys/stat.h>
#include <errno.h>

extern "C" int mknod(const char *pathname, mode_t mode, dev_t dev)
{
    errno = EPERM;
    return -1;
}
