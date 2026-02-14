#include <errno.h>

extern "C" int _getentropy(void *buf, size_t buflen)
{
    errno = ENOSYS;
    return -1;
}
