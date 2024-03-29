#include "syscalls.h"

#include "_gk_gpu.h"
#include <errno.h>
#include "deferred.h"

extern "C" ssize_t GK_GPUEnqueueMessages(const struct gpu_message *msgs, size_t nmsg)
{
    if(!msgs)
    {

    }
    size_t nsent = 0;
    while(nsent < nmsg)
    {
        size_t to_send = nmsg - nsent;
        size_t mw;

        __syscall_gpuenqueue_params p { &msgs[nsent], to_send, &mw };
        auto ret = deferred_call(__syscall_gpuenqueue, &p);
        if(ret == -1)
        {
            if(nsent)
                return (ssize_t)nsent;
            else
                return -1;
        }

        nsent += mw;
    }

    return nsent;
}
