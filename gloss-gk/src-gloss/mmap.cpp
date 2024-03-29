#include "syscalls.h"
#include <sys/mman.h>
#include <errno.h>
#include "deferred.h"
#include <cstring>
#include <unistd.h>

extern "C" void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    if(flags & MAP_FIXED)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }

    // allocate the memory
    void *addrret;
    __syscall_memalloc_params p { len, &addrret };
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

extern "C" int munmap(void *addr, size_t len)
{
    __syscall_memdealloc_params p { len, addr };
    return deferred_call(__syscall_memdealloc, &p);
}
