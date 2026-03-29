#include <sys/sysmacros.h>

extern "C" dev_t makedev(unsigned int maj, unsigned int min)
{
    return ((maj & 0xffU) << 8) | (min & 0xffU);
}

extern "C" unsigned int major(dev_t dev)
{
    return (((unsigned int)dev) >> 8) & 0xffU;
}

extern "C" unsigned int minor(dev_t dev)
{
    return ((unsigned int)dev) & 0xffU;
}
