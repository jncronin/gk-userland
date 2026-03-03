#include "syscalls.h"
#include "_gk_event.h"
#include "deferred.h"
#include "gk.h"
#include <errno.h>

int GK_EventPeek(Event *ev)
{
    if(!ev)
    {
        errno = EINVAL;
        return -1;
    }

    return deferred_call(__syscall_peekevent, ev);
}

int GK_SendEvent(pid_t pid, Event *ev)
{
    if(!ev)
    {
        errno = EINVAL;
        return -1;
    }

    __syscall_sendevent_params p;
    p.pid = pid;
    p.ev = (void *)ev;
    return deferred_call(__syscall_sendevent, &p);
}
