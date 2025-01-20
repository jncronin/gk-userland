#include <sys/mman.h>

// we don't have virtual memory so the following just succeed always
extern "C" int mlock(const void *addr, size_t len)
{
    return 0;
}

extern "C" int mlock2(const void *addr, size_t len, unsigned int flags)
{
    return 0;
}

extern "C" int munlock(const void *addr, size_t len)
{
    return 0;
}

extern "C" int mlockall(int flags)
{
    return 0;
}

extern "C" int munlockall()
{
    return 0;
}
