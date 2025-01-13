#include "syscalls.h"

template <typename T> void cmpxchg(T *ptr, T *oldval, T newval)
{
    static_assert(sizeof(T) <= sizeof(void *));
    __syscall_cmpxchg_params p { .ptr = (void **)ptr, .oldval = (void **)oldval, .newval = (void *)newval, .sz = sizeof(T) };
    int _errno = 0, ret = 0;
    __syscall(__syscall_cmpxchg, &ret, reinterpret_cast<void *>(&p), &_errno);
}
