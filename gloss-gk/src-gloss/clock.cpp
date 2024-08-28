#include <syscalls.h>
#include "deferred.h"

void clock_get_now(struct timespec *tp);
void clock_get_now_monotonic(struct timespec *tp);

extern "C" int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if(!tp)
    {
        errno = EINVAL;
        return -1;
    }

    switch(clk_id)
    {
        case CLOCK_REALTIME:
            clock_get_now(tp);
            return 0;

        case 4: // MONOTONIC
        case 5: // MONOTONIC_RAW
            clock_get_now_monotonic(tp);
            return 0;

        default:
            {
                __syscall_clock_gettime_params p { clk_id, tp };
                return deferred_call(__syscall_clock_gettime, &p);
            }
    }
}
