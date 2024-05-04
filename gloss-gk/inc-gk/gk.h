#ifndef GK_H
#define GK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "_gk_gpu.h"
#include "_gk_event.h"

#include <stdlib.h>

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
int GK_GPUFlipBuffers(void *cmdlist, void **next_buffer);
int GK_GPUFlush(void *cmdlist);
int GK_GPUCleanCache(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride);
int GK_GPUSetScreenMode(void *cmdlist, size_t w, size_t h, unsigned int pf);
int GK_EventPeek(struct Event *ev);

#ifdef __cplusplus
}
#endif

#endif
