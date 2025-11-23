/* Newlib clobbers argc/argv during startup.  Here we restore them. */

#if __GAMEKID__ != 4
#include "syscalls.h"

extern "C" void software_init_hook() __attribute__((naked));

extern "C" void software_init_hook()
{
    __asm volatile
    (
        "mov r0, %0     \n"
        "mov r1, lr     \n"
        "sub sp, #8     \n"
        "mov r2, sp     \n"
        "svc #0         \n"
        "pop {r0-r1}    \n"
        "bx lr          \n"
        :: "r"((int)__syscall_newlibinithook)
    );
}
#endif
