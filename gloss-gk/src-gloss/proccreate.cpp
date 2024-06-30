#include "errno.h"
#include "syscalls.h"
#include "deferred.h"

extern "C" int GK_CreateProcess(const char *fname, const proccreate_t *pcinfo)
{
    if(!fname || !pcinfo)
    {
        errno = EINVAL;
        return -1;
    }

    __syscall_proccreate_params p { .fname = fname, .proc_info = pcinfo };
    return deferred_call(__syscall_proccreate, &p);
}
