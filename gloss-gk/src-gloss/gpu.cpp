#include "syscalls.h"

#include "_gk_gpu.h"
#include <errno.h>
#include "deferred.h"
#include "gk.h"

extern "C" ssize_t GK_GPUEnqueueMessages(const struct gpu_message *msgs, size_t nmsg)
{
    if(!msgs)
    {
        errno = EINVAL;
        return -1;
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

int GK_GPUResetCommandList(void *cmdlist)
{
    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    hdr->__ncmds = 0;
    return 0;
}

int GK_GPUClearScreen(void *cmdlist)
{
    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = ClearScreen;
    msg->dest_addr = 0;
    msg->dest_pf = 0;
    msg->dx = 0;
    msg->dy = 0;
    msg->dw = 0;
    msg->dh = 0;
    msg->dp = 0;
    msg->src_addr_color = 0;
    msg->src_pf = 0;
    return 0;
}

int GK_GPUBlitScreen(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride)
{
    uint32_t spf = 0;
    switch(bpp)
    {
        case 16:
            spf = GK_PIXELFORMAT_RGB565;
            break;
        case 32:
            spf = GK_PIXELFORMAT_ARGB8888;
            break;
        default:
            return -1;
    }

    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = BlitImage;
    msg->dest_addr = 0;
    msg->dest_pf = 0;
    msg->dx = 0;
    msg->dy = 0;
    msg->dw = 0;
    msg->dh = 0;
    msg->dp = 0;
    msg->src_addr_color = (uint32_t)(uintptr_t)src;
    msg->sx = 0;
    msg->sy = 0;
    msg->w = w;
    msg->h = h;
    msg->sp = stride;
    msg->src_pf = spf;

    return 0;
}

int GK_GPUBlitScreenNoBlend(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride)
{
    uint32_t spf = 0;
    switch(bpp)
    {
        case 16:
            spf = GK_PIXELFORMAT_RGB565;
            break;
        case 32:
            spf = GK_PIXELFORMAT_ARGB8888;
            break;
        default:
            return -1;
    }

    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = BlitImageNoBlend;
    msg->dest_addr = 0;
    msg->dest_pf = 0;
    msg->dx = 0;
    msg->dy = 0;
    msg->dw = 0;
    msg->dh = 0;
    msg->dp = 0;
    msg->src_addr_color = (uint32_t)(uintptr_t)src;
    msg->sx = 0;
    msg->sy = 0;
    msg->w = w;
    msg->h = h;
    msg->sp = stride;
    msg->src_pf = spf;

    return 0;
}

int GK_GPUFlush(void *cmdlist)
{
    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);

    size_t ncmds = hdr->__ncmds;
    if(ncmds > hdr->__max_cmds)
        ncmds = hdr->__max_cmds;
    auto ret = GK_GPUEnqueueMessages(msgs, ncmds);

    if((ret >= 0) && ((size_t)ret == hdr->__ncmds))
    {
        // success, now wait for completion
        gpu_message gsignal;
        gsignal.type = SignalThread;
        while(GK_GPUEnqueueMessages(&gsignal, 1) != 1);
    }
    if(ret > 0)
        hdr->__ncmds -= ret;
    return ret;
}

int GK_GPUFlipBuffers(void *cmdlist, void **next_buffer)
{
    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = FlipBuffers;
    msg->dest_addr = (uint32_t)(uintptr_t)next_buffer;
    msg->src_addr_color = 0;

    return 0;
}

int GK_GPUSetScreenMode(void *cmdlist, size_t w, size_t h, unsigned int pf)
{
    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = SetScreenMode;
    msg->w = w;
    msg->h = h;
    msg->dest_pf = pf;

    return 0;
}

int GK_GPUCleanCache(void *cmdlist, const void *src, size_t w, size_t h, size_t bpp, size_t stride)
{
    uint32_t spf = 0;
    switch(bpp)
    {
        case 16:
            spf = GK_PIXELFORMAT_RGB565;
            break;
        case 32:
            spf = GK_PIXELFORMAT_ARGB8888;
            break;
        default:
            return -1;
    }

    auto hdr = reinterpret_cast<__gk_cmd_list_header *>(cmdlist);
    if(hdr->__ncmds >= hdr->__max_cmds)
        return -1;
    auto msgs = reinterpret_cast<gpu_message *>(&hdr[1]);
    auto msg = &msgs[hdr->__ncmds++];

    msg->type = CleanCache;
    msg->dest_addr = (uint32_t)(uintptr_t)src;
    msg->dest_pf = spf;
    msg->dx = 0;
    msg->dy = 0;
    msg->dw = w;
    msg->dh = h;
    msg->dp = stride;
    msg->src_addr_color = (uint32_t)(uintptr_t)src;
    msg->sx = 0;
    msg->sy = 0;
    msg->w = w;
    msg->h = h;
    msg->sp = stride;
    msg->src_pf = spf;

    return 0;
}

int GK_GPUGetScreenMode(size_t *w, size_t *h, unsigned int *pf)
{
    __syscall_getscreenmode_params p;
    p.x = (int *)w;
    p.y = (int *)h;
    p.pf = (int *)pf;
    return deferred_call(__syscall_getscreenmode, &p);
}
