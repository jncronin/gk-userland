#include <syscalls.h>
#include "deferred.h"
#include <errno.h>
#include "gk.h"

int GK_Shutdown(int reboot)
{
    int na = 0;
    return deferred_call(reboot ? __syscall_reboot : __syscall_shutdown, &na);
}
