#ifndef GK_H
#define GK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "_gk_gpu.h"
#include "_gk_event.h"
#include "_gk_proccreate.h"
#include "_gk_scancodes.h"
#include "_gk_memaddrs.h"

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

ssize_t GK_GPUEnqueueMessages(const struct gpu_message *msgs, size_t nmsg);

struct __gk_cmd_list_header
{
    size_t __max_cmds;
    size_t __ncmds;
};

#define GK_GPU_CommandList(name, ncmds) \
    struct __gk_cmd_list_ ## name ## _ ## ncmds { \
        struct __gk_cmd_list_header hdr; \
        struct gpu_message msgs[ncmds]; \
    } name = { { ncmds, 0 } };

int GK_GPUResetCommandList(void *cmdlist);
int GK_GPUClearScreen(void *cmdlist);
int GK_GPUBlitScreen(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride);
int GK_GPUBlitScreenNoBlend(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride);
int GK_GPUBlitScreenNoBlendEx(void *cmdlist, const void *src, size_t x, size_t y, size_t w, size_t h, size_t bpp, size_t stride);
int GK_GPUFlipBuffers(void *cmdlist, void **next_buffer);
int GK_GPUFlipBuffersEx(void *cmdlist, void **next_buffer, void **old_buffer);
int GK_GPUFlush(void *cmdlist);
int GK_GPUCleanCache(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride);
int GK_GPUSetScreenMode(void *cmdlist, size_t w, size_t h, unsigned int pf);
int GK_GPUSetScreenModeEx(void *cmdlist, size_t w, size_t h, unsigned int pf, int refresh);
int GK_ICACHEInvalidate();
int GK_EventPeek(struct Event *ev);
int GK_GPUGetScreenMode(size_t *w, size_t *h, unsigned int *pf);
int GK_GPUGetScreenModeEx(size_t *w, size_t *h, unsigned int *pf, int *refresh);
int GK_WindowSetTitle(const char *title);
int GK_CacheFlush(void *addr, size_t len, int is_exec);
int GK_CreateProcess(const char *fname, const struct proccreate_t *pcinfo, pid_t *pid);

int GK_AudioSetMode(int nchan, int nbits, int freq, size_t buf_size_bytes);
int GK_AudioSetModeEx(int nchan, int nbits, int freq, size_t buf_size_bytes, size_t nbufs);
int GK_AudioEnable(int enable);
int GK_AudioQueueBuffer(const void *buffer, void **next_buffer);
int GK_AudioWaitFree();
int GK_AudioSetFreq(int freq);
int GK_AudioGetBufferPos(size_t *nbufs, size_t *curbuf, size_t *buflen, size_t *bufpos,
    int *nchan, int *nbits, int *freq);

int GK_NemaEnable(void **rb, pthread_mutex_t *eof_mutex);

int GK_SetLED(int led_id, uint32_t color);

int GK_SetSupervisorVisible(int visible, int screen);

uint64_t GK_GetCurUs();

int GK_GetJoystickAxes(int *x, int *y);
int GK_GetTiltAxes(int *x, int *y);

#define GK_AUDIO_MAX_BUFFER_SIZE    (16*1024)

#ifdef __cplusplus
}
#endif

#endif
