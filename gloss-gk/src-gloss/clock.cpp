#include <syscalls.h>
#include "deferred.h"

extern "C" int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    __syscall_clock_gettime_params p { clk_id, tp };
    return deferred_call(__syscall_clock_gettime, &p);
}
