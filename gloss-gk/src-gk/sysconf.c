#include <unistd.h>
#include <errno.h>

long sysconf(int name)
{
    switch(name)
    {
        case _SC_ARG_MAX:
            return 4096;

        case _SC_PAGESIZE:
            return 4096;
        
        default:
            errno = EINVAL;
            return -1;
    }
}
