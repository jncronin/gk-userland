#include <syscalls.h>
#include <sys/time.h>
#include <errno.h>
#include "_gk_memaddrs.h"

#if __GAMEKID__ != 4
#define STM32H747xx
#define CORE_CM7
#include <stm32h7xx.h>

volatile uint64_t *_cur_ms = (volatile uint64_t *)GK_CUR_MS_ADDRESS;
volatile struct timespec *toffset = (volatile struct timespec *)GK_TOFFSET_ADDRESS;

/* these functions try to mirror those in the kernel */
uint64_t clock_cur_us()
{
    /* The basic idea here is to try and read both LPTIM1->CNT and _cur_ms atomically.
        We need to account for the fact that on calling this function:
            1) interrupts may be enabled and so _cur_ms may change as we read LPTIM1->CNT
            2) interrupts may be disabled and so LPTIM1->CNT may rollover without a change in
                _cur_ms - luckily LPTIM1->ISR & ARRM will be set in this instance.
    */

    uint32_t cnt = 0U;
    uint64_t cms = 0U;
    uint32_t isr = 0U;

    while(true)
    {
        cms = *_cur_ms;
        isr = LPTIM1->ISR;
        cnt = LPTIM1->CNT;

        auto isr2 = LPTIM1->ISR;
        auto cms2 = *_cur_ms;

        if(isr == isr2 && cms == cms2) break;
    }

    auto ret = cms + ((isr & LPTIM_ISR_ARRM) ? 1ULL : 0ULL);
    ret *= 1000ULL;
    ret += cnt;
    return ret;
}

void clock_get_timebase(struct timespec *tp)
{
    /* we can't use the spinlock here.  However, given the kernel is
        the only one who can use the spinlock, by simply checking
        the value hasn't changed between reads we can be happy we
        don't have a half-updated value because any syscall would
        complete between two checks of the value */
    
    if(!tp) return;

    while(true)
    {
        struct timespec tp2;

        tp->tv_sec = toffset->tv_sec;
        tp->tv_nsec = toffset->tv_nsec;

        tp2.tv_sec = toffset->tv_sec;
        tp2.tv_nsec = toffset->tv_nsec;
        
        if(tp->tv_sec == tp2.tv_sec &&
            tp->tv_nsec == tp2.tv_nsec)
        {
            return;
        }
    }
}

void clock_get_now(struct timespec *tp)
{
    if(!tp) return;
    clock_get_timebase(tp);
    auto curt = clock_cur_us();

    auto cur_ns = (curt % 1000000ULL) * 1000ULL;
    auto cur_s = curt / 1000000ULL;

    tp->tv_nsec += cur_ns;
    while(tp->tv_nsec >= 1000000000)
    {
        tp->tv_sec++;
        tp->tv_nsec -= 1000000000;
    }
    tp->tv_sec += cur_s;
}

void clock_get_now_monotonic(struct timespec *tp)
{
    if(!tp) return;
    auto curt = clock_cur_us();

    auto cur_ns = (curt % 1000000ULL) * 1000ULL;
    auto cur_s = curt / 1000000ULL;

    tp->tv_nsec = cur_ns;
    tp->tv_sec = cur_s;
}

extern "C" int _gettimeofday(struct timeval *tv, timezone *tz)
{
    if(!tv)
    {
        errno = EINVAL;
        return -1;
    }

    timespec ts;
    clock_get_now(&ts);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;

    if(tz)
    {
        // just return UTC for now
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = DST_NONE;
    }
    return 0;
}

extern "C" uint64_t GK_GetCurUs()
{
    return clock_cur_us();
}

#else

#define _cur_s ((volatile uint64_t *)(GK_CUR_S_ADDRESS))
#define _cur_sc_ns_val ((volatile uint32_t *)(GK_TIM3 + 0x24ULL))
#define _tim_precision_ns ((volatile uint64_t *)(GK_TIM_PRECISION_NS_ADDRESS))
#define toffset ((volatile timespec *)(GK_TOFFSET_ADDRESS))

timespec clock_cur()
{
    while(true)
    {
        uint64_t _s_a = *_cur_s;
        uint64_t _cur_sc_ns = *_cur_sc_ns_val;
        uint64_t _s_b = *_cur_s;

        if(_s_a == _s_b)
        {
            timespec ret;
            ret.tv_nsec = _cur_sc_ns * *_tim_precision_ns;
            ret.tv_sec = _s_a;
            return ret;
        }
    }
}

void clock_get_timebase(struct timespec *tp)
{
    /* we can't use the spinlock here.  However, given the kernel is
        the only one who can use the spinlock, by simply checking
        the value hasn't changed between reads we can be happy we
        don't have a half-updated value because any syscall would
        complete between two checks of the value */
    
    if(!tp) return;

    while(true)
    {
        struct timespec tp2;

        tp->tv_sec = toffset->tv_sec;
        tp->tv_nsec = toffset->tv_nsec;

        tp2.tv_sec = toffset->tv_sec;
        tp2.tv_nsec = toffset->tv_nsec;
        
        if(tp->tv_sec == tp2.tv_sec &&
            tp->tv_nsec == tp2.tv_nsec)
        {
            return;
        }
    }
}

void clock_get_now(struct timespec *tp)
{
    if(!tp) return;
    clock_get_timebase(tp);
    auto cc = clock_cur();

    tp->tv_nsec += cc.tv_nsec;
    while(tp->tv_nsec >= 1000000000)
    {
        tp->tv_sec++;
        tp->tv_nsec -= 1000000000;
    }
    tp->tv_sec += cc.tv_sec;
}

void clock_get_now_monotonic(struct timespec *tp)
{
    if(!tp) return;
    *tp = clock_cur();
}

uint64_t clock_cur_us()
{
    auto cc = clock_cur();
    return (cc.tv_sec * 1000000LL) + (cc.tv_nsec / 1000LL);
}

extern "C" uint64_t GK_GetCurUs()
{
    return clock_cur_us();
}

extern "C" int _gettimeofday(struct timeval *tv, timezone *tz)
{
    if(!tv)
    {
        errno = EINVAL;
        return -1;
    }

    timespec ts;
    clock_get_now(&ts);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;

    if(tz)
    {
        // just return UTC for now
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = DST_NONE;
    }
    return 0;
}

#endif
