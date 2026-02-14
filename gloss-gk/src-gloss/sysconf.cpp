#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <errno.h>
#include "_gk_memaddrs.h"

extern "C" long sysconf(int name)
{
    switch(name)
    {
        case _SC_ARG_MAX:
            return 4096;

        case _SC_PAGESIZE:
#if __GAMEKID__ >= 4
            return GK_KERNEL_INFO->page_size;
#else
            return 4096;
#endif

        case _SC_NPROCESSORS_ONLN:
        case _SC_NPROCESSORS_CONF:
#if __GAMEKID__ >= 4
            return GK_KERNEL_INFO->ncores;
#else
            return 1;
#endif

        case _SC_OPEN_MAX:
            return 16;
        
        default:
            errno = EINVAL;
            return -1;
    }
}

extern "C" int getrlimit(int resource, struct rlimit *rlim)
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

extern "C" int getrusage(int who, struct rusage *usage)
{
    errno = ENOTSUP;
    return -1;
}

extern "C" int getpagesize()
{
    return (int)sysconf(_SC_PAGE_SIZE);
}

extern "C" int setrlimit(int resource, const rlimit *new_limit)
{
    return 0;
}
