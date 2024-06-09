#include <unistd.h>
#include <syscalls.h>
#include <deferred.h>

extern "C" int usleep(useconds_t usec)
{
    usec /= 1000;
    return deferred_call(__syscall_sleep_ms, &usec);
}

extern "C" unsigned int sleep(unsigned int seconds)
{
    seconds *= 1000;
    deferred_call(__syscall_sleep_ms, &seconds);
    return 0;
}
