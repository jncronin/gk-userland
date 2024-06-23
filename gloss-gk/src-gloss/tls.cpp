#include "syscalls.h"

extern "C" void *__aeabi_read_tp()
{
    void *ptr;
    __syscall(__syscall_read_tp, (void *)&ptr, (void *)&ptr, nullptr);
    return ptr;
}
