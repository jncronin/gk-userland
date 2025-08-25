#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_GK

#include "../SDL_sysrender.h"
#include "SDL_video.h"
#include "../../video/SDL_sysvideo.h"
#include "SDL_hints.h"

#include "gk.h"
#include "_gk_gpu.h"
#include <sys/mman.h>

#include <nema_core.h>

// define the following as weak to allow usage when we don't link libnemagfx
__attribute__((weak)) void nema_set_clip(int32_t, int32_t, uint32_t, uint32_t) {}
__attribute__((weak)) void nema_ext_hold_assert(uint32_t, int32_t) {}
__attribute__((weak)) void nema_cl_unbind() {}
__attribute__((weak)) void nema_cl_submit(nema_cmdlist_t *) {}
__attribute__((weak)) void nema_cl_bind_circular(nema_cmdlist_t *) {}
__attribute__((weak)) void nema_cl_bind(nema_cmdlist_t *) {}
__attribute__((weak)) void nema_cl_rewind(nema_cmdlist_t *) {}
__attribute__((weak)) void nema_bind_dst_tex(uintptr_t, uint32_t, uint32_t,
    nema_tex_format_t, int32_t) {}
__attribute__((weak)) void nema_bind_src_tex(uintptr_t, uint32_t, uint32_t,
    nema_tex_format_t, int32_t, nema_tex_mode_t) {}
__attribute__((weak)) void nema_fill_rect(int, int, int, int, uint32_t) {}
__attribute__((weak)) void nema_draw_line(int, int, int, int, uint32_t) {}
__attribute__((weak)) void nema_blit_rect_fit(int, int, int, int) {}
__attribute__((weak)) uint32_t nema_rgba(unsigned char, unsigned char, unsigned char, unsigned char) { return 0; } 
__attribute__((weak)) int nema_init() { return -1; }
__attribute__((weak)) int nema_rb_init(nema_ringbuffer_t *, int) { return -1; }
__attribute__((weak)) void nema_ext_hold_irq_enable(uint32_t) {}
__attribute__((weak)) nema_cmdlist_t nema_cl_create() { nema_cmdlist_t ret = { 0 }; return ret; }
__attribute__((weak)) nema_cmdlist_t nema_cl_create_sized(int) { nema_cmdlist_t ret = { 0 }; return ret; }
__attribute__((weak)) void nema_ext_hold_enable(uint32_t) {}

typedef struct GKNema_RenderData_t
{
    pthread_mutex_t nema_m;
    nema_cmdlist_t nema_cl;
    size_t w, h;
    void *fb;
    nema_tex_format_t fb_pf;
    uint32_t draw_col;
} GKNema_RenderData;

typedef struct GKNema_TextureData_t
{
    void *addr;
    int is_mmap;
    nema_tex_format_t pf;
} GKNema_TextureData;

nema_ringbuffer_t nema_rb;

static nema_tex_format_t sdlpf_to_nemapf(uint32_t sdlpf)
{
    switch(sdlpf)
    {
        case SDL_PIXELFORMAT_XRGB8888:
            return NEMA_XRGB8888;
        case SDL_PIXELFORMAT_RGB24:
            return NEMA_RGB24;
        case SDL_PIXELFORMAT_ARGB8888:
            return NEMA_RGBA8888;
        case SDL_PIXELFORMAT_RGB565:
            return NEMA_RGB565;
        default:
            printf("unknown sdlpf: %u\n", sdlpf);
            return NEMA_ARGB8888;
    }
}

static nema_tex_format_t gkpf_to_nemapf(unsigned int gkpf)
{
    switch(gkpf)
    {
        case GK_PIXELFORMAT_ARGB8888:
            return NEMA_RGBA8888;
        case GK_PIXELFORMAT_RGB888:
            return NEMA_RGB24;
        case GK_PIXELFORMAT_RGB565:
            return NEMA_RGB565;
        case GK_PIXELFORMAT_L8:
            return NEMA_L8;
        default:
            printf("unknown gkpf: %u\n", gkpf);
            return 0;
    }
}

static unsigned int nemapf_to_gkpf(nema_tex_format_t nemapf)
{
    switch(nemapf)
    {
        case NEMA_ARGB8888:
        case NEMA_RGBA8888:
            return GK_PIXELFORMAT_ARGB8888;
        case NEMA_RGB24:
            return GK_PIXELFORMAT_RGB888;
        case NEMA_RGB565:
            return GK_PIXELFORMAT_RGB565;
        case NEMA_L8:
            return GK_PIXELFORMAT_L8;
        default:
            printf("unknown nemapf: %d\n", (int)nemapf);
            return 0;
    }
}

static int nemapf_to_pixelsize(nema_tex_format_t nemapf)
{
    switch(nemapf)
    {
        case NEMA_ARGB8888:
        case NEMA_RGBA8888:
            return 4;
        case NEMA_XRGB8888:
            return 4;
        case NEMA_RGB24:
            return 3;
        case NEMA_RGB565:
            return 2;
        case NEMA_L8:
            return 1;
        default:
            printf("unknown nemapf: %d\n", (int)nemapf);
            return 0;
    }
}

static void nema_start_frame(GKNema_RenderData *rd)
{
    nema_cl_bind_circular(&rd->nema_cl);
    //nema_cl_rewind(&rd->nema_cl);

    nema_bind_dst_tex((uintptr_t)rd->fb, rd->w, rd->h, rd->fb_pf, -1);
    nema_set_clip(0, 0, rd->w, rd->h);
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 0, rd->w, rd->h, nema_rgba(0, 0, 0, 255));

    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
}

static void nema_end_frame(GKNema_RenderData *rd)
{
    //nema_ext_hold_assert(0, 0);
    //nema_cl_unbind();
    nema_cl_submit(&rd->nema_cl);
    nema_cl_wait(&rd->nema_cl);
}

int32_t nema_sys_init()
{
    int ret = nema_rb_init(&nema_rb, 1);
    if(ret < 0) return ret;
    return 0;
}


static int GKNema_QueueSetViewport(SDL_Renderer *r, SDL_RenderCommand *cmd)
{
    nema_set_clip(cmd->data.viewport.rect.x,
        cmd->data.viewport.rect.y,
        cmd->data.viewport.rect.w,
        cmd->data.viewport.rect.h);
    return 0;
}

static int GKNema_QueueSetDrawColor(SDL_Renderer *r, SDL_RenderCommand *cmd)
{
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    rd->draw_col = nema_rgba(cmd->data.color.r,
        cmd->data.color.g,
        cmd->data.color.b,
        cmd->data.color.a);
    return 0;
}

static int GKNema_QueueDrawPoints(SDL_Renderer *r, SDL_RenderCommand *cmd, const SDL_FPoint *points,
                           int count)
{
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    for(int i = 0; i < count; i++)
    {
        nema_draw_line((int)points[i].x, (int)points[i].y, (int)points[i].x, (int)points[i].y,
            rd->draw_col);
    }
    return 0;
}

static int GKNema_QueueDrawLines(SDL_Renderer *r, SDL_RenderCommand *cmd,
    const SDL_FPoint *points, int count)
{
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    for(int i = 0; i < count - 1; i++)
    {
        nema_draw_line((int)points[i].x, (int)points[i].y, (int)points[i + 1].x, (int)points[i + 1].y,
            rd->draw_col);
    }
    return 0;
}

static int GKNema_QueueFillRects(SDL_Renderer *r, SDL_RenderCommand *cmd,
    const SDL_FRect *rects, int count)
{
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    for(int i = 0; i < count; i++)
    {
        nema_fill_rect((int)rects[i].x, (int)rects[i].y, (int)rects[i].w, (int)rects[i].h,
            rd->draw_col);
    }
    return 0;
}

static int GKNema_QueueCopy(SDL_Renderer *r, SDL_RenderCommand *cmd, SDL_Texture *texture,
                     const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    GKNema_TextureData *td = (GKNema_TextureData *)texture->driverdata;

    // srcrect can be partial texture here - set up the texture appropriately
    {
        int pwidth = nemapf_to_pixelsize(td->pf);
        uintptr_t srcaddr = (uintptr_t)td->addr +
            srcrect->y * texture->pitch +
            srcrect->x * pwidth;
        nema_bind_src_tex(srcaddr, srcrect->w, srcrect->h, td->pf, texture->pitch,
            NEMA_FILTER_PS | NEMA_TEX_CLAMP);
    }
    nema_blit_rect_fit(dstrect->x, dstrect->y, dstrect->w, dstrect->h);
    return 0;
}

static int GKNema_QueueCopyEx(SDL_Renderer *r, SDL_RenderCommand *cmd, SDL_Texture *texture,
                       const SDL_Rect *srcrect, const SDL_FRect *dstrect,
                       const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip, float scale_x, float scale_y)
{
    // TODO: support rotation - use nema_blit_quad_fit with transformed dest points, 
    // flipped if necessary
    GKNema_TextureData *td = (GKNema_TextureData *)texture->driverdata;

    // srcrect can be partial texture here - set up the texture appropriately
    {
        int pwidth = nemapf_to_pixelsize(td->pf);
        uintptr_t srcaddr = (uintptr_t)td->addr +
            srcrect->y * texture->pitch +
            srcrect->x * pwidth;
        nema_bind_src_tex(srcaddr, srcrect->w, srcrect->h, td->pf, texture->pitch,
            NEMA_FILTER_PS | NEMA_TEX_CLAMP);
    }
    nema_blit_rect_fit(dstrect->x, dstrect->y, dstrect->w, dstrect->h);
    return 0;    
}

static int GKNema_RunCommandQueue(SDL_Renderer *r, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    return 0;
}

static int GKNema_RenderPresent(SDL_Renderer *r)
{
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    nema_end_frame(rd);

    // wait here?
    {
        GK_GPU_CommandList(cmds, 2);
        GK_GPUFlipBuffers(&cmds, &rd->fb);
        GK_GPUFlush(&cmds);
    }

    nema_start_frame(rd);

    return 0;
}

static int GKNema_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GKNema_TextureData *td;
    void *tmem = MAP_FAILED;
    int is_mmap = 1;

    texture->pitch = (((texture->w * SDL_BYTESPERPIXEL(texture->format)) + 3) & ~3);

    printf("GKNema_CreateTexture: %d x %d (pformat: %u, bpp: %d), access: %u\n",
        texture->w, texture->h, texture->format, texture->pitch, texture->access);

    if(texture->access != SDL_TEXTUREACCESS_STATIC)
    {
        // For NEMA, we can just have static textures in the heap
        int flags = MAP_PRIVATE | MAP_ANON;
        if(texture->access == SDL_TEXTUREACCESS_STREAMING)
            flags |= MAP_SYNC;

        tmem = mmap(NULL, texture->h * texture->pitch, PROT_READ | PROT_WRITE, 
            flags, 0, 0);
    }

    if(tmem == MAP_FAILED)
    {
        // Probably out of MPU slots - it's first come first served on GK
        tmem = malloc(texture->h * texture->pitch);
        is_mmap = 0;

        if(!tmem)
        {
            printf("malloc(%d x %d = %d) failed\n", texture->h, texture->pitch,
                texture->h * texture->pitch);
        }
    }
    if(!tmem)
    {
        return -1;
    }

    td = (GKNema_TextureData *)SDL_calloc(1, sizeof(GKNema_TextureData));
    if(!td)
    {
        if(is_mmap)
        {
            munmap(tmem, texture->h * texture->pitch);
            return -1;
        }
        else
        {
            free(tmem);
            return -1;
        }
    }
    td->addr = tmem;
    td->is_mmap = is_mmap;
    td->pf = sdlpf_to_nemapf(texture->format);
    texture->driverdata = td;

    return 0;
}

static void GKNema_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GKNema_TextureData *td = (GKNema_TextureData *)texture->driverdata;
    if(td)
    {
        if(td->is_mmap)
        {
            munmap(td->addr, texture->pitch * texture->h);
        }
        else
        {
            free(td->addr);
        }
        SDL_free(td);
        texture->driverdata = NULL;
    }
}

static int GKNema_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, void **pixels, int *pitch)
{
    char *dest = ((GKNema_TextureData *)texture->driverdata)->addr;

    if(rect && (rect->x || rect->y))
    {
        unsigned int pixel_size = nemapf_to_pixelsize(sdlpf_to_nemapf(texture->format));
        dest = (dest + texture->pitch * rect->y) + rect->x + pixel_size;
    }

    if(pixels)
    {
        *pixels = dest;
    }

    if(pitch)
    {
        *pitch = texture->pitch;
    }

    return 0;
}

static void GKNema_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
}

static int GKNema_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, const void *pixels, int pitch)
{
    struct gpu_message gmsgs[3];

    char *dest = ((GKNema_TextureData *)texture->driverdata)->addr;
    uint32_t tpf = nemapf_to_gkpf(((GKNema_TextureData *)texture->driverdata)->pf);
    
    gmsgs[0].type = CleanCache;
    gmsgs[0].dest_addr = (uint32_t)(uintptr_t)pixels;
    gmsgs[0].dx = 0;
    gmsgs[0].dy = 0;
    gmsgs[0].dw = rect->w;
    gmsgs[0].dh = rect->h;
    gmsgs[0].dp = pitch;
    gmsgs[0].dest_pf = tpf;

    gmsgs[1].type = BlitImage;
    gmsgs[1].dest_addr = (uint32_t)(uintptr_t)dest;
    gmsgs[1].dest_pf = tpf;
    gmsgs[1].dx = rect->x;
    gmsgs[1].dy = rect->y;
    gmsgs[1].dw = rect->w;
    gmsgs[1].dh = rect->h;
    gmsgs[1].dp = texture->pitch;
    gmsgs[1].w = rect->w;
    gmsgs[1].h = rect->h;
    gmsgs[1].src_addr_color = (uint32_t)(uintptr_t)pixels;
    gmsgs[1].src_pf = tpf;
    gmsgs[1].sx = 0;
    gmsgs[1].sy = 0;
    gmsgs[1].sp = pitch;
    
    gmsgs[2].type = SignalThread;

    GK_GPUEnqueueMessages(gmsgs, 3);

    return 0;
}

static SDL_Renderer *GKNema_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    GKNema_RenderData *data;
    pthread_mutex_t nema_m;

    GK_NemaEnable((void **)&nema_rb, &nema_m);
    if(nema_init() < 0)
    {
        return NULL;
    }

    // we have been linked with nemagfx, therefore provide a valid renderer
    renderer = (SDL_Renderer *)SDL_calloc(1, sizeof(*renderer));
    if(renderer == NULL)
    {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (GKNema_RenderData *)SDL_calloc(1, sizeof(*data));
    if(data == NULL)
    {
        SDL_free(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    data->w = window->w;
    data->h = window->h;
    data->nema_m = nema_m;

    //nema_ext_hold_enable(0);
    //nema_ext_hold_irq_enable(0);
    data->nema_cl = nema_cl_create_sized(32*1024);

    // Flip screens once to get current fb addr
    {
        GK_GPU_CommandList(cmds, 2);
        GK_GPUFlipBuffers(&cmds, &data->fb);
        GK_GPUFlush(&cmds);
    }
    
    // Get pixel format - just of screen for now (off-screen rendering unsupported)
    {
        unsigned int gkpf;
        uint32_t gkpf_to_pformat(unsigned int gkpf);

        GK_GPUGetScreenMode(NULL, NULL, &gkpf);
        data->fb_pf = gkpf_to_nemapf(gkpf);
    }

#if DEBUG_GK
    printf("GKNema_CreateRenderer\n");
#endif

    // fill in required functions
    renderer->QueueSetViewport = GKNema_QueueSetViewport;
    renderer->QueueSetDrawColor = GKNema_QueueSetDrawColor;
    renderer->QueueDrawPoints = GKNema_QueueDrawPoints;
    renderer->QueueDrawLines = GKNema_QueueDrawLines;
    renderer->QueueFillRects = GKNema_QueueFillRects;
    renderer->QueueCopy = GKNema_QueueCopy;
    renderer->QueueCopyEx = GKNema_QueueCopyEx;
    renderer->RunCommandQueue = GKNema_RunCommandQueue;
    renderer->RenderPresent = GKNema_RenderPresent;
    renderer->CreateTexture = GKNema_CreateTexture;
    renderer->UpdateTexture = GKNema_UpdateTexture;
    renderer->DestroyTexture = GKNema_DestroyTexture;
    renderer->LockTexture = GKNema_LockTexture;
    renderer->UnlockTexture = GKNema_UnlockTexture;

    renderer->info = GKNema_RenderDriver.info;
    renderer->driverdata = data;
    renderer->window = window;

    return renderer;
}

SDL_RenderDriver GKNema_RenderDriver = {
    .CreateRenderer = GKNema_CreateRenderer,
    .info = {
        .name = "GKNema renderer",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .num_texture_formats = 4,   // TODO: can we use more?
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_ARGB8888,
            [1] = SDL_PIXELFORMAT_RGB888,
            [2] = SDL_PIXELFORMAT_RGB565,
            [3] = SDL_PIXELFORMAT_INDEX8
        },        
        .max_texture_width = 2048,
        .max_texture_height = 2048
    }
};

#endif
