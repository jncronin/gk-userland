#include <signal.h>
#include <errno.h>

extern "C" int sigaction(int signum,
    const struct sigaction *act,
    struct sigaction *oldact)
{
    if(!oldact)
    {
        errno = EFAULT;
        return -1;
    }

    oldact->sa_handler = SIG_DFL;
    
    errno = EINVAL;
    return -1;
}
