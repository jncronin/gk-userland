#include "syscalls.h"
#include <sys/mman.h>
#include <errno.h>
#include "deferred.h"
#include "_gk_memaddrs.h"
#include <string.h>
#include <unistd.h>

#if __GAMEKID__ >= 4
extern "C" void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    if(offset < 0)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }
    if(offset & (GK_KERNEL_INFO->page_size - 1))
    {
        errno = EINVAL;
        return MAP_FAILED;
    }
    void *addrret = addr;
    __syscall_mmapv4_params p {
        len,
        &addrret,
        (flags & MAP_SYNC) ? 1 : 0,
        (prot & PROT_READ) ? 1 : 0,
        (prot & PROT_WRITE) ? 1 : 0,
        (prot & PROT_EXEC) ? 1 : 0,
        (flags & MAP_ANON) ? -1 : fd,
        (flags & MAP_FIXED) ? 1 : 0,
        (size_t)offset
    };

    auto ret = deferred_call(__syscall_mmapv4, &p);
    if(ret != 0)
    {
        return MAP_FAILED;
    }

    return addrret;
}

#else
extern "C" void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    if(flags & MAP_FIXED)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }

    // allocate the memory
    void *addrret;
    __syscall_memalloc_params p { len, &addrret, flags & MAP_SYNC };
    auto ret = deferred_call(__syscall_memalloc, &p);
    if(ret != 0)
    {
        return MAP_FAILED;
    }

    if(flags & MAP_ANON)
    {
        // required to be zero
        memset(addrret, 0, len);
    }
    else
    {
        // back with file
        ret = read(fd, addrret, len);
        if(ret != 0)
        {
            munmap(addrret, len);
            return MAP_FAILED;
        }

        // TODO: tell kernel what file is backing so it can sync appropriately
    }

    // Set protection
    __syscall_setprot_params p2 { addrret,
        (prot & PROT_READ) ? 1 : 0,
        (prot & PROT_WRITE) ? 1 : 0,
        (prot & PROT_EXEC) ? 1 : 0
    };
    ret = deferred_call(__syscall_setprot, &p2);
    if(ret != 0)
    {
        munmap(addrret, len);
        return MAP_FAILED;
    }

    return addrret;
}
#endif

extern "C" int munmap(void *addr, size_t len)
{
    __syscall_memdealloc_params p { len, addr };
    return deferred_call(__syscall_memdealloc, &p);
}

extern "C" int mprotect(void *addr, size_t len, int prot)
{
    __syscall_setprot_params p2 { addr,
        (prot & PROT_READ) ? 1 : 0,
        (prot & PROT_WRITE) ? 1 : 0,
        (prot & PROT_EXEC) ? 1 : 0
    };
    return deferred_call(__syscall_setprot, &p2);
}
