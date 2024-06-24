#include "syscalls.h"

extern "C" void *__aeabi_read_tp_c()
{
    void *ptr;
    __syscall(__syscall_read_tp, (void *)&ptr, (void *)&ptr, nullptr);
    return ptr;
}

/* __aeabi_read_tp is not allow to clobber any registers except r0 */
extern "C" void *__aeabi_read_tp() __attribute__((naked));

extern "C" void *__aeabi_read_tp()
{
    __asm__ volatile (
        "push { r1, r2, r3, lr }    \n"
        "bl __aeabi_read_tp_c       \n"
        "pop { r1, r2, r3, lr }     \n"
        "bx lr                      \n"
        ::: "memory");
}
