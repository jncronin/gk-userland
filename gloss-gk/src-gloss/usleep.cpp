#include <unistd.h>
#include <syscalls.h>
#include <deferred.h>

extern "C" int usleep(useconds_t usec)
{
    usec /= 1000;
    return deferred_call(__syscall_sleep_ms, &usec);
}
