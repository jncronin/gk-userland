#include "deferred.h"
#include "syscalls.h"
#include "gk.h"

int GK_CacheFlush(void *addr, size_t len, int is_exec)
{
    struct __syscall_cacheflush_params p { addr, len, is_exec };
    return deferred_call(__syscall_cacheflush, &p);
}
