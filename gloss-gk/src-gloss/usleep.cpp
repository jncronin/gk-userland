#define _POSIX_MONOTONIC_CLOCK

#include <syscalls.h>
#include <unistd.h>
#include <deferred.h>
#include <time.h>

extern uint64_t clock_cur_us();

extern "C" int usleep(useconds_t usec)
{
    if(usec == 0) return 0;
    if(usec < 1000)
    {
        // busy wait
        auto now = clock_cur_us();
        while(clock_cur_us() < (now + (uint64_t)usec))
        {
            __asm__ volatile("yield\n" ::: "memory");
        }
        return 0;
    }
    else
    {
        uint64_t _usec = usec;
        return deferred_call(__syscall_sleep_us, &_usec);
    }
}

extern "C" unsigned int sleep(unsigned int seconds)
{
    if(seconds == 0) return 0;
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

    if(duration->tv_sec == 0 && duration->tv_nsec < 1000000)
    {
        // <1 ms just do busy wait
        usleep(duration->tv_nsec / 1000 + 1);
        return 0;
    }
    
    // else use sleep functions
    auto start_time = clock_cur_us();

    // use sleep_us
    uint64_t usec = (uint64_t)(duration->tv_sec * 1000) +
        (uint64_t)(duration->tv_nsec / 1000);
    deferred_call(__syscall_sleep_us, &usec);
    auto now_time = clock_cur_us();
    if(now_time < (start_time + usec))
    {
        if(rem)
        {
            auto diff = start_time + usec - now_time;
            rem->tv_sec = diff / 1000000ULL;
            rem->tv_nsec = (diff % 1000000ULL) * 1000ULL;
        }
        errno = EINTR;
        return -1;
    }
    return 0;
}
