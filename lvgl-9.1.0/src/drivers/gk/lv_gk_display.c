#include "../../core/lv_refr.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"
#include "../../lv_init.h"

#include <gk.h>

struct gk_display_data
{
    struct gpu_message *cmsgs;
    unsigned int cmsgs_size;
    unsigned int cmsgs_count;
    void *next_buffer;
    size_t w;
    size_t h;
    size_t stride;
};

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);
struct gpu_message *get_next_msg(struct gk_display_data *dd);
static void release_disp_cb(lv_event_t * e);
static uint32_t get_ticks();

static const int nmsgs_start = 8;

lv_display_t *lv_gk_display_create()
{
    size_t w, h, stride;
    unsigned int gkpf;
    GK_GPUGetScreenMode(&w, &h, &gkpf);

    struct gk_display_data *dd = lv_malloc_zeroed(sizeof(struct gk_display_data));
    if(dd == NULL) return NULL;
    dd->cmsgs_size = nmsgs_start;
    dd->cmsgs_count = 0;
    dd->cmsgs = lv_malloc_zeroed(dd->cmsgs_size * sizeof(struct gpu_message));
    if(dd->cmsgs == NULL)
    {
        lv_free(dd);
        return NULL;
    }

    lv_display_t *disp = lv_display_create(w, h);
    if(disp == NULL)
    {
        lv_free(dd->cmsgs);
        lv_free(dd);
        return NULL;
    }

    switch(gkpf)
    {
        case GK_PIXELFORMAT_ARGB8888:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
            stride = w * 4;
            break;

        case GK_PIXELFORMAT_RGB888:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB888);
            stride = w * 3;
            break;

        case GK_PIXELFORMAT_RGB565:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
            stride = w * 2;
            break;

        case GK_PIXELFORMAT_L8:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_I8);
            stride = w;
            break;

        default:
            lv_display_delete(disp);
            return NULL;
    }

    lv_display_set_resolution(disp, w, h);
    
    {
        /* Get first framebuffer */
        void *fb;
        GK_GPU_CommandList(gmsgs, 1);
        GK_GPUFlipBuffers(&gmsgs, &fb);
        GK_GPUFlush(&gmsgs);

        lv_display_set_buffers(disp, fb, NULL, h * stride, LV_DISPLAY_RENDER_MODE_DIRECT);
    }

    dd->w = w;
    dd->h = h;
    dd->stride = stride;

    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_add_event_cb(disp, release_disp_cb, LV_EVENT_DELETE, disp);
    lv_display_set_driver_data(disp, dd);

    lv_tick_set_cb(get_ticks);
    
    return disp;
}

void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    struct gk_display_data *dd = lv_display_get_driver_data(disp);

    if(dd->cmsgs_count == 0)
    {
        // Add a flip buffers message first
        struct gpu_message *gmsg = get_next_msg(dd);
        if(!gmsg)
            return;
        gmsg->type = FlipBuffers;
        gmsg->dest_addr = (uint32_t)(uintptr_t)&dd->next_buffer;
        gmsg->src_addr_color = 0;
    }

    /* Add area, if any */
    if(area)
    {
        struct gpu_message *cgmsg = get_next_msg(dd);
        if(!cgmsg)
            return;
        cgmsg->type = BlitImage;
        cgmsg->dest_addr = 0;
        cgmsg->dest_pf = 0;
        cgmsg->dp = 0;
        cgmsg->dx = area->x1;
        cgmsg->dy = area->y1;
        cgmsg->dw = area->x2 - area->x1;
        cgmsg->dh = area->y2 - area->y1;
        cgmsg->src_addr_color = 0;
        cgmsg->src_pf = 0;
        cgmsg->sp = 0;
        cgmsg->sx = cgmsg->dx;
        cgmsg->sy = cgmsg->dy;
        cgmsg->w = cgmsg->dw;
        cgmsg->h = cgmsg->dh;
    }
    else
    {
        /* not sure if this occurs, but just copy all anyway */
        struct gpu_message *cgmsg = get_next_msg(dd);
        if(!cgmsg)
            return;
        cgmsg->type = BlitImage;
        cgmsg->dest_addr = 0;
        cgmsg->dest_pf = 0;
        cgmsg->dp = 0;
        cgmsg->dx = 0;
        cgmsg->dy = 0;
        cgmsg->dw = dd->w;
        cgmsg->dh = dd->h;
        cgmsg->src_addr_color = 0;
        cgmsg->src_pf = 0;
        cgmsg->sp = 0;
        cgmsg->sx = 0;
        cgmsg->sy = 0;
        cgmsg->w = dd->w;
        cgmsg->h = dd->h;
    }

    /* If last update in the frame, add a signal thread and return */
    if(lv_display_flush_is_last(disp))
    {
        struct gpu_message *cgmsg = get_next_msg(dd);
        if(!cgmsg)
            return;
        cgmsg->type = SignalThread;
        cgmsg->dest_addr = 0;
        cgmsg->src_addr_color = 0;

        GK_GPUEnqueueMessages(dd->cmsgs, dd->cmsgs_count);

        /* reset ready for next frame */
        dd->cmsgs_count = 0;

        lv_display_set_buffers(disp, dd->next_buffer, NULL,
            dd->stride * dd->h, LV_DISPLAY_RENDER_MODE_DIRECT);
    }

    lv_display_flush_ready(disp);
}

struct gpu_message *get_next_msg(struct gk_display_data *dd)
{
    /* may need to realloc the buffer here */
    while(dd->cmsgs_count >= dd->cmsgs_size)
    {
        dd->cmsgs = lv_realloc(dd->cmsgs, dd->cmsgs_size * 2);
        dd->cmsgs_size *= 2;
    }
    if(dd->cmsgs == NULL)
    {
        return NULL;
    }
    return &dd->cmsgs[dd->cmsgs_count++];
}

void release_disp_cb(lv_event_t *e)
{
    lv_display_t *disp = (lv_display_t *)lv_event_get_user_data(e);
    struct gk_display_data *dd = lv_display_get_driver_data(disp);
    if(dd)
    {
        if(dd->cmsgs)
            lv_free(dd->cmsgs);
        lv_free(dd);
    }
    lv_display_set_driver_data(disp, NULL);
}

uint32_t get_ticks()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((ts.tv_nsec / 1000000ULL) + (ts.tv_sec * 1000ULL));
}
