#include "syscalls.h"

#if __GAMEKID__ == 4

// can use atomics here
#if 0
template <typename T> void cmpxchg(T *ptr, T *oldval, T newval)
{
    if constexpr (sizeof(T) == 8)
    {
        __asm__ volatile
        (
            "ldxr "
        )
    }
    else if constexpr(sizeof(T) == 4)
    {

    }
    else
    {
        static_assert(false, "invalid object size for cmpxchg - need 4 or 8 bytes");
    }
}


void cmpxchg(int *ptr, int *oldval, int newval);
void cmpxchg(unsigned int *ptr, unsigned int *oldval, unsigned int newval);
void cmpxchg(long *ptr, long *oldval, long newval);
void cmpxchg(unsigned long *ptr, unsigned long *oldval, unsigned long newval);
#endif

template <typename T> static inline void cmpxchg(T *ptr, T *oldval, T newval)
{
    __atomic_compare_exchange_n(ptr, oldval, newval, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
#else
template <typename T> void cmpxchg(T *ptr, T *oldval, T newval)
{
    static_assert(sizeof(T) <= sizeof(void *));
    __syscall_cmpxchg_params p { .ptr = (void **)ptr, .oldval = (void **)oldval, .newval = (void *)newval, .sz = sizeof(T) };
    int _errno = 0, ret = 0;
    __syscall(__syscall_cmpxchg, &ret, reinterpret_cast<void *>(&p), &_errno);
}
#endif
