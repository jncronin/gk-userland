#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <errno.h>

long sysconf(int name)
{
    switch(name)
    {
        case _SC_ARG_MAX:
            return 4096;

        case _SC_PAGESIZE:
            return 4096;

        case _SC_OPEN_MAX:
            return 16;
        
        default:
            errno = EINVAL;
            return -1;
    }
}

int getrlimit(int resource, struct rlimit *rlim)
{
    switch(resource)
    {
        case RLIMIT_NOFILE:
            rlim->rlim_max = 16;
            return 0;

        default:
            return -1;
    }
}

int getrusage(int who, struct rusage *usage)
{
    errno = ENOTSUP;
    return -1;
}
