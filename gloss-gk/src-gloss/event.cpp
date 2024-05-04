#include "syscalls.h"
#include "_gk_event.h"
#include "deferred.h"
#include "gk.h"
#include <errno.h>

int GK_PeekEvent(Event *ev)
{
    if(!ev)
    {
        errno = EINVAL;
        return -1;
    }

    return deferred_call(__syscall_peekevent, ev);
}
