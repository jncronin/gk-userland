#include <syscalls.h>
#include "deferred.h"
#include <sys/timeb.h>

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

extern "C" int ftime(struct timeb *tp)
{
    if(!tp)
    {
        errno = EINVAL;
        return -1;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    tp->time = ts.tv_sec;
    tp->millitm = ts.tv_nsec / 1000000;
    tp->timezone = 0;
    tp->dstflag = 0;

    return 0;
}
