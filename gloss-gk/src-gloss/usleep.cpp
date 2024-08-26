#define _POSIX_MONOTONIC_CLOCK

#include <syscalls.h>
#include <unistd.h>
#include <deferred.h>
#include <time.h>

extern "C" int usleep(useconds_t usec)
{
    return deferred_call(__syscall_sleep_us, &usec);
}

extern "C" unsigned int sleep(unsigned int seconds)
{
    seconds *= 1000;
    deferred_call(__syscall_sleep_ms, &seconds);
    return 0;
}

extern "C" int clock_gettime(clockid_t clk_id, struct timespec *tp);

static timespec operator+(const timespec &a, const timespec &b)
{
    timespec ret = a;
    ret.tv_nsec += b.tv_nsec;
    while(ret.tv_nsec >= 1000000000)
    {
        ret.tv_nsec -= 1000000000;
        ret.tv_sec += 1;
    }
    ret.tv_sec += b.tv_sec;
    return ret;
}

static timespec operator-(const timespec &a, const timespec &b)
{
    timespec ret = a;
    ret.tv_nsec -= b.tv_nsec;
    while(ret.tv_nsec < 0)
    {
        ret.tv_nsec += 1000000000;
        ret.tv_sec -= 1;
    }
    ret.tv_sec -= b.tv_sec;
    return ret;
}

static bool operator<(const timespec &a, const timespec &b)
{
    if(a.tv_sec < b.tv_sec) return true;
    if(a.tv_sec > b.tv_sec) return false;
    return a.tv_nsec < b.tv_nsec;
}

static bool operator>(const timespec &a, const timespec &b)
{
    if(a.tv_sec > b.tv_sec) return true;
    if(a.tv_sec < b.tv_sec) return false;
    return a.tv_nsec > b.tv_nsec;
}

extern "C" int nanosleep(const timespec *duration, timespec *rem)
{
    if(!duration)
    {
        errno = EINVAL;
        return -1;
    }
    timespec startt, exp_endt, act_endt;
    clock_gettime(CLOCK_MONOTONIC, &startt);
    exp_endt = startt + *duration;

    if(duration->tv_sec == 0 && duration->tv_nsec < 2000000)
    {
        // <2 ms just do busy wait
        do
        {
            clock_gettime(CLOCK_MONOTONIC, &act_endt);
        } while(act_endt < exp_endt);
    }
    else
    {
        // use sleep_ms
        unsigned int msec = (unsigned int)(duration->tv_nsec / 1000000) +
            (unsigned int)(duration->tv_sec * 1000);
        deferred_call(__syscall_sleep_ms, &msec);
        clock_gettime(CLOCK_MONOTONIC, &act_endt);
    }
    if(act_endt < exp_endt)
    {
        if(rem)
            *rem = exp_endt - act_endt;
        errno = EINTR;
        return -1;
    }
    else
    {
        return 0;
    }
}
