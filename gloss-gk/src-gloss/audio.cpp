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

int GK_AudioSetModeEx(int nchan, int nbits, int freq, size_t buf_size_bytes, size_t nbufs)
{
    __syscall_audiosetmodeex_params p { .nchan = nchan, .nbits = nbits, .freq = freq,
        .buf_size_bytes = buf_size_bytes, .nbufs = nbufs };
    return deferred_call(__syscall_audiosetmodeex, &p);
}

int GK_AudioGetBufferPos(size_t *nbufs, size_t *curbuf, size_t *buflen, size_t *bufpos,
    int *nchan, int *nbits, int *freq)
{
    __syscall_audiogetbufferpos_params p { .nbufs = nbufs, .curbuf = curbuf, .buflen = buflen,
        .bufpos = bufpos, .nchan = nchan, .nbits = nbits, .freq = freq };
    return deferred_call(__syscall_audiogetbufferpos, &p);
}

int GK_AudioEnable(int enable)
{
    return deferred_call(__syscall_audioenable, (void*)(intptr_t)enable);
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
    return deferred_call(__syscall_audiosetfreq, (void*)(intptr_t)freq);
}
