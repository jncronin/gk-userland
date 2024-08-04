#include "syscalls.h"
#include "deferred.h"
#include <sys/times.h>

extern "C" clock_t _times(struct tms *buf)
{
    clock_t ret;
    __syscall_times_params p { buf, &ret };
    deferred_call(__syscall_times, &p);
    return ret;
}
