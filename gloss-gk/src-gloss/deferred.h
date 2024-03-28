#ifndef DEFERRED_H
#define DEFERRED_H

#include <errno.h>
#include <syscalls.h>

static inline int deferred_return(int ret, int _errno)
{
    if(ret == -1)
    {
        errno = _errno;
        return ret;
    }
    if(ret == -2)
    {
        // deferred return
        struct WaitSimpleSignal_params wssp;
        while(1)
        {
            uint32_t wss;
            __syscall(WaitSimpleSignal, &wss, &wssp, NULL);
            if(wss)
                break;
        }
        if(wssp.ival1 == -1)
            errno = wssp.ival2;
        return wssp.ival1;
    }
    return ret;
}

/* The following only valid in C++ code */
#ifdef __cplusplus

template<typename T> int deferred_call(syscall_no sno, T arg)
{
    int _errno = 0, ret = 0;
    __syscall(sno, &ret, reinterpret_cast<void *>(arg), &_errno);
    return deferred_return(ret, _errno);
}

template<typename T> int deferred_call_with_retry(syscall_no sno, T arg)
{
    while(true)
    {
        int _errno = 0, ret = 0;
        __syscall(sno, &ret, reinterpret_cast<void *>(arg), &_errno);
        auto ret = deferred_return(ret, _errno);
        if(ret != -2)
            return ret;
    }
}


#endif

#endif