#include <unistd.h>
#include <errno.h>

extern "C" int chown(const char *pathname, uid_t owner, gid_t group)
{
    errno = EPERM;
    return -1;
}

extern "C" int fchown(int fd, uid_t owner, gid_t group)
{
    errno = EPERM;
    return -1;
}

extern "C" int lchown(const char *pathname, uid_t owner, gid_t group)
{
    errno = EPERM;
    return -1;
}
