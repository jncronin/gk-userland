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

#define DEBUG_GKNEMA    0

// Nema pixel formats


//  NEMA_RGBA8888 = R@0, G@1, B@2, A@3 - same as SDL_ABGR8888 / SDL_RGBA32 - used for PNG textures
//  NEMA_RGBA4444 = A@0-low, B@0-high, G@1-low, R@1-high - NOT the same as GK_ARGB4444
//  NEMA_BGRA8888 = B@0, G@1, R@2, A@3 - this is the same as GK_ARGB8888 and SDL_ARGB8888 / SDL_BGRA32, used for framebuffer


// Cache word size of the ICACHE is 32 bytes - presumably the minimum granularity Nema supports
//  Therefore, align all textures to this
static const uintptr_t tex_align = 32;

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
__attribute__((weak)) nema_cmdlist_t nema_cl_create_prealloc(nema_buffer_t *) { nema_cmdlist_t ret = { 0 }; return ret; }
__attribute__((weak)) void nema_ext_hold_enable(uint32_t) {}

typedef struct GKNema_RenderData_t
{
    pthread_mutex_t nema_m;
    nema_cmdlist_t nema_cl_a, nema_cl_b;
    nema_cmdlist_t *nema_cl;
    size_t w, h;
    void *fb;
    nema_tex_format_t fb_pf;
    uint32_t draw_col;
    void *ones_64bytes;
    void *zeros_64bytes;
} GKNema_RenderData;

typedef struct GKNema_TextureData_t
{
    void *addr;
    void *free_addr;
    int is_mmap;
    nema_tex_format_t pf;
} GKNema_TextureData;

nema_ringbuffer_t nema_rb;

static nema_tex_format_t sdlpf_to_nemapf(uint32_t sdlpf)
{
    switch(sdlpf)
    {
        case SDL_PIXELFORMAT_XRGB8888:
            return NEMA_RGBX8888;
        case SDL_PIXELFORMAT_RGB24:
            return NEMA_RGB24;
        case SDL_PIXELFORMAT_ARGB8888:
            return NEMA_BGRA8888;
        case SDL_PIXELFORMAT_ABGR8888:
            return NEMA_RGBA8888;
        case SDL_PIXELFORMAT_ARGB1555:
            return NEMA_RGBA5551;
        case SDL_PIXELFORMAT_ARGB4444:
            return NEMA_RGBA4444;
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
            return NEMA_BGRA8888;
        case GK_PIXELFORMAT_RGB888:
            return NEMA_RGB24;
        case GK_PIXELFORMAT_RGB565:
            return NEMA_RGB565;
        case GK_PIXELFORMAT_ARGB4444:
            return NEMA_BGRA4444;
        case GK_PIXELFORMAT_ARGB1555:
            return NEMA_RGBA5551;
        case GK_PIXELFORMAT_L8:
            return NEMA_L8;
        default:
            printf("unknown gkpf: %u\n", gkpf);
            return 0;
    }
}

#if 0
// this is just used for DMA2D blits without blending, therefore just need to get BPP right
static unsigned int nemapf_to_gkpf(nema_tex_format_t nemapf)
{
    switch(nemapf)
    {
        case NEMA_ARGB8888:
        case NEMA_RGBA8888:
        case NEMA_BGRA8888:
            return GK_PIXELFORMAT_ARGB8888;
        case NEMA_BGRA4444:
            return GK_PIXELFORMAT_ARGB4444;
        case NEMA_RGBA5551:
            return GK_PIXELFORMAT_ARGB1555;
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
#endif

static int nemapf_to_pixelsize(nema_tex_format_t nemapf)
{
    switch(nemapf)
    {
        case NEMA_ARGB8888:
        case NEMA_RGBA8888:
        case NEMA_BGRA8888:
            return 4;
        case NEMA_XRGB8888:
            return 4;
        case NEMA_RGB24:
            return 3;
        case NEMA_RGB565:
        case NEMA_RGBA4444:
        case NEMA_RGBA5551:
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
    nema_cl_bind_circular(rd->nema_cl);
    //nema_cl_rewind(&rd->nema_cl);

    nema_bind_dst_tex((uintptr_t)rd->fb, rd->w, rd->h, rd->fb_pf, -1);
    nema_set_clip(0, 0, rd->w, rd->h);
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 0, rd->w, rd->h, nema_rgba(0, 0, 0, 255));

    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_set_blend_blit(nema_blending_mode(NEMA_BF_SRCALPHA, NEMA_BF_INVSRCALPHA,
        NEMA_BLOP_MODULATE_A | NEMA_BLOP_MODULATE_RGB));
}

static void nema_end_frame(GKNema_RenderData *rd)
{
    //  NEMA_RGBA8888 = R@0, G@1, B@2, A@3
    //  NEMA_RGBA4444 = A@0-low, B@0-high, G@1-low, R@1-high
    //  NEMA_BGRA8888 = B@0, G@1, R@2, A@3 - this is the same as GK_ARGB8888 and SDL_ARGB8888 = SDL_BGRA32
    //nema_bind_dst_tex((uintptr_t)rd->fb, rd->w, rd->h, NEMA_RGBA4444, -1);
#if DEBUG_GKNEMA
    nema_set_clip(0, 0, rd->w, rd->h);
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 0, 50, 50, nema_rgba(255, 0, 0, 127));
    nema_fill_rect(50, 0, 50, 50, nema_rgba(0, 255, 0, 127));
    nema_fill_rect(100, 0, 50, 50, nema_rgba(0, 0, 255, 127));
    //nema_clear(nema_rgba(250, 200, 100, 50));
    printf("nema pf: %u\n", rd->fb_pf);
#endif

    nema_ext_hold_assert(0, 0);
    //printf("nema: cl size: %d, offset: %d\n", rd->nema_cl.size, rd->nema_cl.offset); // CL typically 8k
    //nema_cl_unbind();
    nema_cl_submit(rd->nema_cl);

    // TODO: remove once gk is doing triple buffering for us
    nema_cl_wait(rd->nema_cl);

    //uint8_t *fb_b = (uint8_t *)rd->fb;
    //printf("fb: %u, %u, %u, %u\n", fb_b[0], fb_b[1], fb_b[2], fb_b[3]);

    if(rd->nema_cl == &rd->nema_cl_a)
        rd->nema_cl = &rd->nema_cl_b;
    else
        rd->nema_cl = &rd->nema_cl_a;
}

int32_t nema_sys_init()
{
    int ret = nema_rb_init(&nema_rb, 1);
    if(ret < 0) return ret;
    return 0;
}


static int GKNema_QueueSetViewport(SDL_Renderer *r, SDL_RenderCommand *cmd)
{
#if DEBUG_GKNEMA
    printf("nema: setviewport(%d, %d, %d, %d)\n", cmd->data.viewport.rect.x,
        cmd->data.viewport.rect.y,
        cmd->data.viewport.rect.w,
        cmd->data.viewport.rect.h);
#endif
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
    nema_set_blend_fill(NEMA_BL_SIMPLE);
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
    nema_set_blend_fill(NEMA_BL_SIMPLE);
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
    // nema_fill_rect seems to not do anything sometimes - unclear why
    //  instead, use a pre-populated texture of ones and modulate with draw_col then blit
    GKNema_RenderData *rd = (GKNema_RenderData *)r->driverdata;
    //nema_set_clip(0, 0, rd->w, rd->h);
    //nema_set_blend_fill(NEMA_BL_SRC);
    for(int i = 0; i < count; i++)
    {
        nema_bind_src_tex((uintptr_t)rd->ones_64bytes,
            4, 4, NEMA_RGBA8888, 16,
            NEMA_FILTER_PS | NEMA_TEX_CLAMP);
        nema_set_const_color(rd->draw_col);
        nema_set_blend_blit(nema_blending_mode(NEMA_BF_SRCALPHA, NEMA_BF_INVSRCALPHA,
            NEMA_BLOP_MODULATE_A | NEMA_BLOP_MODULATE_RGB));
        nema_blit_rect_fit(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        // printf("rect: %f x %f @ %f,%f, col %08x\n", rects[i].w, rects[i].h, rects[i].x, rects[i].y, rd->draw_col);
        //nema_fill_rect_f(rects[i].x, rects[i].y, rects[i].w, rects[i].h,
        //    rd->draw_col);
    }
    return 0;
}

static int GKNema_QueueCopy(SDL_Renderer *r, SDL_RenderCommand *cmd, SDL_Texture *texture,
                     const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    GKNema_TextureData *td = (GKNema_TextureData *)texture->driverdata;
    uint8_t tr, tg, tb, ta;
    SDL_GetTextureColorMod(texture, &tr, &tg, &tb);
    SDL_GetTextureAlphaMod(texture, &ta);

    // srcrect can be partial texture here - set up the texture appropriately
    {
        int pwidth = nemapf_to_pixelsize(td->pf);
        uintptr_t srcaddr = (uintptr_t)td->addr +
            srcrect->y * texture->pitch +
            srcrect->x * pwidth;
        nema_bind_src_tex(srcaddr, srcrect->w, srcrect->h, td->pf, texture->pitch,
            NEMA_FILTER_PS | NEMA_TEX_CLAMP);
    }
    nema_set_const_color(nema_rgba(tr, tg, tb, ta));
    nema_set_blend_blit(nema_blending_mode(NEMA_BF_SRCALPHA, NEMA_BF_INVSRCALPHA,
        NEMA_BLOP_MODULATE_A | NEMA_BLOP_MODULATE_RGB));
    nema_blit_rect_fit(dstrect->x, dstrect->y, dstrect->w, dstrect->h);
    return 0;
}

static int GKNema_QueueCopyEx(SDL_Renderer *r, SDL_RenderCommand *cmd, SDL_Texture *texture,
                       const SDL_Rect *srcrect, const SDL_FRect *dstrect,
                       const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip, float scale_x, float scale_y)
{
    GKNema_TextureData *td = (GKNema_TextureData *)texture->driverdata;
    uint8_t tr, tg, tb, ta;
    float points[8];

    // support flipping
#define TL_X (dstrect->x)
#define TL_Y (dstrect->y)
#define TR_X (dstrect->x + dstrect->w)
#define TR_Y (dstrect->y)
#define BL_X (dstrect->x)
#define BL_Y (dstrect->y + dstrect->h)
#define BR_X (dstrect->x + dstrect->w)
#define BR_Y (dstrect->y + dstrect->h)

    switch((int)flip)
    {
        case SDL_FLIP_HORIZONTAL:
            points[0] = TR_X;
            points[1] = TR_Y;
            points[2] = TL_X;
            points[3] = TL_Y;
            points[4] = BL_X;
            points[5] = BL_Y;
            points[6] = BR_X;
            points[7] = BR_Y;
            break;

        case SDL_FLIP_VERTICAL:
            points[0] = BL_X;
            points[1] = BL_Y;
            points[2] = BR_X;
            points[3] = BR_Y;
            points[4] = TR_X;
            points[5] = TR_Y;
            points[6] = TL_X;
            points[7] = TL_Y;
            break;

        case SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL:
            points[0] = BR_X;
            points[1] = BR_Y;
            points[2] = BL_X;
            points[3] = BL_Y;
            points[4] = TL_X;
            points[5] = TL_Y;
            points[6] = TR_X;
            points[7] = TR_Y;
            break;

        default:
            points[0] = TL_X;
            points[1] = TL_Y;
            points[2] = TR_X;
            points[3] = TR_Y;
            points[4] = BR_X;
            points[5] = BR_Y;
            points[6] = BL_X;
            points[7] = BL_Y;
            break;
    }

    // TODO: support rotation - transform points[] based upon translate back to centre, rotate, translate forward


    SDL_GetTextureColorMod(texture, &tr, &tg, &tb);
    SDL_GetTextureAlphaMod(texture, &ta);

    // srcrect can be partial texture here - set up the texture appropriately
    {
        int pwidth = nemapf_to_pixelsize(td->pf);
        uintptr_t srcaddr = (uintptr_t)td->addr +
            srcrect->y * texture->pitch +
            srcrect->x * pwidth;
        nema_bind_src_tex(srcaddr, srcrect->w, srcrect->h, td->pf, texture->pitch,
            NEMA_FILTER_PS | NEMA_TEX_CLAMP);
    }

    nema_set_const_color(nema_rgba(tr, tg, tb, ta));
    nema_set_blend_blit(nema_blending_mode(NEMA_BF_SRCALPHA, NEMA_BF_INVSRCALPHA,
        NEMA_BLOP_MODULATE_A | NEMA_BLOP_MODULATE_RGB));
    nema_blit_quad_fit(points[0], points[1], points[2], points[3],
        points[4], points[5], points[6], points[7]);
    //nema_blit_rect_fit(dstrect->x, dstrect->y, dstrect->w, dstrect->h);
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
    void *free_addr = NULL;
    int is_mmap = 1;

    texture->pitch = (((texture->w * SDL_BYTESPERPIXEL(texture->format)) + (tex_align - 1)) & ~(tex_align - 1));

#if DEBUG_GKNEMA
    printf("GKNema_CreateTexture: %d x %d (pformat: %u, bpp: %d), access: %u\n",
        texture->w, texture->h, texture->format, texture->pitch, texture->access);
#endif

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
        free_addr = malloc(texture->h * texture->pitch + tex_align);
        is_mmap = 0;

        if(!free_addr)
        {
            printf("malloc(%d x %d = %d) failed\n", texture->h, texture->pitch,
                texture->h * texture->pitch);
            tmem = NULL;
        }
        else
        {
            tmem = (void *)(((uintptr_t)free_addr + (tex_align - 1)) & ~(tex_align - 1));
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
            free(free_addr);
            return -1;
        }
    }
    td->addr = tmem;
    td->is_mmap = is_mmap;
    td->free_addr = free_addr;
    td->pf = sdlpf_to_nemapf(texture->format);
    texture->driverdata = td;

#if DEBUG_GKNEMA
    printf("Nema: allocated texture @ %08x\n", (uint32_t)(uintptr_t)td->addr);
#endif

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
            free(td->free_addr);
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
    GK_ICACHEInvalidate();
}

static int GKNema_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, const void *pixels, int pitch)
{
    char *dest = ((GKNema_TextureData *)texture->driverdata)->addr;

#if DEBUG_GKNEMA
    printf("GKNema_UpdateTexture: from %08x (%d x %d, pitch = %d, pf = %d), to %08x (%d,%d, %d x %d, pitch=%d)\n",
        (uint32_t)(uintptr_t)pixels, rect->w, rect->h, pitch, tpf,
        (uint32_t)(uintptr_t)dest,
        rect->x, rect->y, rect->w, rect->h,
        texture->pitch);
#endif

    {
        int bpp = SDL_BYTESPERPIXEL(texture->format);
        uint8_t *from = ((uint8_t *)pixels) + rect->y * pitch + rect->x * bpp;
        uint8_t *to = (uint8_t *)dest;
        GK_GPU_CommandList(cc, 1);

        for(int y = 0; y < rect->h; y++)
        {
            memcpy(to, from, rect->w * bpp);
            from += pitch;
            to += texture->pitch;
        }

        GK_GPUCleanCache(&cc, dest, rect->w, rect->h, bpp*4, texture->pitch);
        GK_GPUFlush(&cc);
        GK_ICACHEInvalidate();
    }

    return 0;
}

static SDL_Renderer *GKNema_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    GKNema_RenderData *data;
    pthread_mutex_t nema_m;
    nema_buffer_t buf_cl_a, buf_cl_b;
    void *ones, *zeros;

    GK_NemaEnable((void **)&nema_rb, &nema_m, (void **)&buf_cl_a, (void **)&buf_cl_b,
        (void **)&ones, (void **)&zeros);
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
    data->nema_cl_a = nema_cl_create_prealloc(&buf_cl_a);
    data->nema_cl_b = nema_cl_create_prealloc(&buf_cl_b);
    data->nema_cl = &data->nema_cl_a;
    data->ones_64bytes = ones;
    data->zeros_64bytes = zeros;

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

#if DEBUG_GKNEMA
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
        .num_texture_formats = 5,   // TODO: can we use more?
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_ARGB8888,
            [1] = SDL_PIXELFORMAT_ABGR8888,
            [2] = SDL_PIXELFORMAT_RGB888,
            [3] = SDL_PIXELFORMAT_RGB565,
            [4] = SDL_PIXELFORMAT_INDEX8,
        },        
        .max_texture_width = 2048,
        .max_texture_height = 2048
    }
};

#endif
