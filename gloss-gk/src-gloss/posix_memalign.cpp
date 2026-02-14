#include <stdlib.h>
#include <malloc.h>
#include <errno.h>

extern "C" int posix_memalign(void **memptr, size_t align, size_t size)
{
    if(!memptr)
    {
        return EINVAL;
    }

    auto ret = memalign(align, size);

    if(ret == nullptr)
    {
        return ENOMEM;
    }

    *memptr = ret;
    return 0;
}
