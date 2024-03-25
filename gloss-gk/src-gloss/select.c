#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

int select(int nfds, fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    struct timeval *timeout)
{
    // we don't support select(!0, ...), but do support select(0, ...) as a sleep function

    if(!timeout)
    {
        errno = EINVAL;
        return -1;
    }
    if(nfds)
    {
        errno = ENOMEM;
        return -1;
    }

    usleep(timeout->tv_sec * 1000000 + timeout->tv_usec);
}

