#include "sys/sysinfo.h"
#include "_gk_memaddrs.h"
#include <errno.h>

int sysinfo(struct sysinfo *i)
{
    errno = ENOTSUP;
    return -1;
}

int get_nprocs_conf()
{
    return GK_KERNEL_INFO->ncores;
}

int get_nprocs()
{
    return GK_KERNEL_INFO->ncores;
}

long get_phys_pages()
{
    errno = ENOTSUP;
    return -1;
}

long get_avphys_pages()
{
    errno = ENOTSUP;
    return -1;
}
