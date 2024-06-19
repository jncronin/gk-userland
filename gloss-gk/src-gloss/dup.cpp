#include <unistd.h>
#include <errno.h>

extern "C" int dup2(int oldfd, int newfd)
{
    errno = EBADF;
    return -1;
}
