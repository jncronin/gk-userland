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

extern "C" int sigaltstack(const stack_t *ss, stack_t *oldss)
{
    return 0;
}

extern "C" unsigned int alarm(unsigned int seconds)
{
    return 0;
}
