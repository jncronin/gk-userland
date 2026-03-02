#include "gk.h"
#include "syscalls.h"
#include "deferred.h"

extern "C" int GK_WifiEnable(int enable)
{
    return deferred_call(__syscall_wifienable, (void*)(intptr_t)enable);
}

extern "C" int GK_RawSDEnable(int enable)
{
    return deferred_call(__syscall_rawsdenable, (void*)(intptr_t)enable);
}
