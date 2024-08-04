#include "syscalls.h"

extern "C" void *__aeabi_read_tp_c()
{
    void *ptr;
    __syscall(__syscall_read_tp, (void *)&ptr, (void *)&ptr, nullptr);
    return ptr;
}

/* __aeabi_read_tp is not allow to clobber any registers except r0 */
extern "C" void *__aeabi_read_tp() __attribute__((naked));

#if 0
extern "C" void *__aeabi_read_tp()
{
    __asm__ volatile (
        "push { r1, r2, r3, lr }    \n"
        "bl __aeabi_read_tp_c       \n"
        "pop { r1, r2, r3, lr }     \n"
        "bx lr                      \n"
        ::: "memory");
}
#endif

/* handle quick tp load - only supports core 0 for now */
#define xstr(s) str(s)
#define str(s) #s

extern "C" void *__aeabi_read_tp()
{
    __asm__ volatile (
        "ldr r0, [pc, #4]           \n"
        "ldr r0, [r0]               \n"
        "bx lr                      \n"
        ".short 0x0000              \n"
        ".word " xstr(GK_TLS_POINTER_ADDRESS) " \n"
        ::: "memory");
}
