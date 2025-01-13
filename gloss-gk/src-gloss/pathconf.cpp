#include <unistd.h>
#include <limits.h>
#include <errno.h>

extern "C" long pathconf(const char *path, int name)
{
    switch(name)
    {
        case _PC_LINK_MAX:
            return LINK_MAX;
        case _PC_MAX_CANON:
            return MAX_CANON;
        case _PC_MAX_INPUT:
            return MAX_INPUT;
        case _PC_NAME_MAX:
            return NAME_MAX;
        case _PC_PATH_MAX:
            return PATH_MAX;
        case _PC_PIPE_BUF:
            return PIPE_BUF;
        case _PC_CHOWN_RESTRICTED:
            return 0;
        case _PC_NO_TRUNC:
            return 1;
        case _PC_VDISABLE:
            return 0;
        default:
            errno = EINVAL;
            return -1;
    }
}