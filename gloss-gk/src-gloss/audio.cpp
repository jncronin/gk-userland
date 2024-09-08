#define _POSIX_READER_WRITER_LOCKS
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES

#include "gk.h"
#include "syscalls.h"
#include "deferred.h"

int GK_AudioSetMode(int nchan, int nbits, int freq, size_t buf_size_bytes)
{
    __syscall_audiosetmode_params p { .nchan = nchan, .nbits = nbits, .freq = freq,
        .buf_size_bytes = buf_size_bytes };
    return deferred_call(__syscall_audiosetmode, &p);
}

int GK_AudioEnable(int enable)
{
    return deferred_call(__syscall_audioenable, (void*)enable);
}

int GK_AudioQueueBuffer(const void *buffer, void **next_buffer)
{
    __syscall_audioqueuebuffer_params p { .buffer = buffer, .next_buffer = next_buffer };
    return deferred_call(__syscall_audioqueuebuffer, &p);
}

int GK_AudioWaitFree()
{
    return deferred_call(__syscall_audiowaitfree, (void*)NULL);
}

int GK_AudioSetFreq(int freq)
{
    return deferred_call(__syscall_audiosetfreq, (void*)freq);
}
