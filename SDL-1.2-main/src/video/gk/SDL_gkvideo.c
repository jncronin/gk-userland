#define SDL_PrivateGLData osmesa_context
#define private_hwdata GKSurfaceData

#include "SDL_config.h"
#include "SDL.h"
#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "gk.h"
#include <sys/mman.h>
#include <GL/osmesa.h>
#include <unistd.h>
#include <string.h>

struct GKSurfaceData
{
    int in_cache;
};

/* keymap from scancodes (i.e. SDL2 keys) to SDL1 SDLKey */
static SDLKey keymap[GK_NUM_SCANCODES];

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this

/* Initialization/Query functions */
static int GK_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **GK_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *GK_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int GK_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void GK_VideoQuit(_THIS);

/* Hardware surface functions */
static int GK_AllocHWSurface(_THIS, SDL_Surface *surface);
static int GK_LockHWSurface(_THIS, SDL_Surface *surface);
static int GK_FlipHWSurface(_THIS, SDL_Surface *surface);
static void GK_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void GK_FreeHWSurface(_THIS, SDL_Surface *surface);

static void GK_UpdateRects(_THIS, int numrects, SDL_Rect *rect);
static int GK_AccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
                        SDL_Surface *dst, SDL_Rect *dstrect);
static int GK_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst);
static int GK_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color);

static void GK_InitOSKeymap(_THIS);
static void GK_PumpEvents(_THIS);

static void GK_SetCaption(_THIS, const char *caption, const char *icon_caption);

static int GK_GL_MakeCurrent(_THIS);
static void GK_GL_SwapWindow(_THIS);
static int GK_GL_LoadLibrary(_THIS, const char *path);
static void *GK_GL_GetProcAddress(_THIS, const char *name);

void OSMesaNemaEndFrame(OSMesaContext ctx);
void OSMesaEnableNema(OSMesaContext ctx, GLboolean enable);

static int GK_Available(void)
{
    return 1;
}

static void GK_DeleteDevice(SDL_VideoDevice *device)
{
    SDL_free(device);
}

static SDL_VideoDevice *GK_CreateDevice(int devindex)
{
    SDL_VideoDevice *device = 0;

    device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
    if(device)
    {
        SDL_memset(device, 0, sizeof(*device));
        device->hidden = 0;
    }
    if(device == NULL)
    {
        SDL_OutOfMemory();
        return 0;
    }

    device->VideoInit = GK_VideoInit;
    device->ListModes = GK_ListModes;
    device->SetVideoMode = GK_SetVideoMode;
    device->CreateYUVOverlay = NULL;
    device->SetColors = GK_SetColors;
    device->UpdateRects = GK_UpdateRects;
    device->VideoQuit = GK_VideoQuit;
    device->AllocHWSurface = GK_AllocHWSurface;
    device->CheckHWBlit = GK_CheckHWBlit;
    device->FillHWRect = GK_FillHWRect;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->LockHWSurface = GK_LockHWSurface;
    device->UnlockHWSurface = GK_UnlockHWSurface;
    device->FlipHWSurface = GK_FlipHWSurface;
    device->FreeHWSurface = GK_FreeHWSurface;
    device->SetCaption = GK_SetCaption;
    device->SetIcon = NULL;
    device->IconifyWindow = NULL;
    device->GrabInput = NULL;
    device->GetWMInfo = NULL;
    device->InitOSKeymap = GK_InitOSKeymap;
    device->PumpEvents = GK_PumpEvents;
    device->info.blit_hw = 1;
    device->info.blit_hw_A = 1;
    device->info.blit_sw = 1;
    device->info.blit_sw_A = 1;
    device->info.blit_fill = 1;
    device->free = GK_DeleteDevice;

    device->GL_LoadLibrary = GK_GL_LoadLibrary;
    device->GL_GetProcAddress = GK_GL_GetProcAddress;
    device->GL_MakeCurrent = GK_GL_MakeCurrent;
    device->GL_SwapBuffers = GK_GL_SwapWindow;
    device->gl_data = NULL;

    return device;
}

VideoBootStrap GK_bootstrap = {
    "gk", "SDL GK video driver",
    GK_Available, GK_CreateDevice
};

static unsigned int pformat_to_gkpf(const SDL_PixelFormat *vformat)
{
    switch(vformat->BitsPerPixel)
    {
        case 32:
            return GK_PIXELFORMAT_ARGB8888;
        case 24:
            return GK_PIXELFORMAT_RGB888;
        case 16:
            return GK_PIXELFORMAT_RGB565;
        case 8:
            return GK_PIXELFORMAT_L8;
        default:
            return 0;
    }
}

static void fill_pformat(SDL_PixelFormat *vformat, unsigned int gk_pf)
{
    switch(gk_pf)
    {
        case GK_PIXELFORMAT_ARGB8888:
            vformat->BitsPerPixel = 32;
            vformat->BytesPerPixel = 4;
            vformat->Amask = 0xff000000U;
            vformat->Rmask = 0x00ff0000U;
            vformat->Gmask = 0x0000ff00U;
            vformat->Bmask = 0x000000ffU;
            break;
        case GK_PIXELFORMAT_RGB888:
            vformat->BitsPerPixel = 24;
            vformat->BytesPerPixel = 3;
            vformat->Rmask = 0x00ff0000U;
            vformat->Gmask = 0x0000ff00U;
            vformat->Bmask = 0x000000ffU;
            break;
        case GK_PIXELFORMAT_RGB565:
            vformat->BitsPerPixel = 16;
            vformat->BytesPerPixel = 2;
            vformat->Rmask = 0x0000f800;
            vformat->Gmask = 0x000007e0;
            vformat->Bmask = 0x0000001f;
            break;
    }
}

int GK_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
    size_t w, h;
    unsigned int gk_pf;
    GK_GPUGetScreenMode(&w, &h, &gk_pf);

    fill_pformat(vformat, gk_pf);
    this->info.hw_available = 1;
    this->info.blit_hw = 1;
    this->info.blit_hw_A = 1;
    this->info.blit_sw = 1;
    this->info.blit_sw_A = 1;
    this->info.blit_fill = 1;
    this->info.current_w = w;
    this->info.current_h = h;
    this->info.vfmt = NULL;
    this->info.video_mem = 0;


    return 0;
}

SDL_Rect mode640x480 = { 0, 0, 640, 480 };
SDL_Rect mode320x240 = { 0, 0, 320, 240 };
SDL_Rect mode160x120 = { 0, 0, 160, 120 };

SDL_Rect *modes[] = { &mode640x480, &mode320x240, &mode160x120, NULL };

SDL_Rect **GK_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
    return modes;
}

SDL_Surface *GK_SetVideoMode(_THIS, SDL_Surface *current,
    int width, int height, int bpp, Uint32 flags)
{
    SDL_PixelFormat pf;
    unsigned int gk_pf;

    /* Sanitize request */
#if __GAMEKID__ == 4
    if(width > GK_KERNEL_INFO->max_screen_width)
    {
        width = GK_KERNEL_INFO->max_screen_width;
    }
    if(height > GK_KERNEL_INFO->max_screen_height)
    {
        height = GK_KERNEL_INFO->max_screen_height;
    }
    width = (width + 3) & ~3;
    height = (height + 3) & ~3;
#else
    if(width > 320)
    {
        width = 640;
        height = 480;
    }
    else if(width > 160)
    {
        width = 320;
        height = 240;
    }
    else
    {
        width = 160;
        height = 120;
    }
#endif

    if(bpp > 24)
    {
        gk_pf = GK_PIXELFORMAT_ARGB8888;
    }
    else if(bpp > 16)
    {
        gk_pf = GK_PIXELFORMAT_RGB888;
    }
    else
    {
        gk_pf = GK_PIXELFORMAT_RGB565;
    }
    fill_pformat(&pf, gk_pf);

    // Set mode
    {
        GK_GPU_CommandList(gmsg, 2);
        GK_GPUSetScreenMode(&gmsg, width, height, gk_pf);
        GK_GPUFlipBuffers(&gmsg, &current->pixels);
        GK_GPUFlush(&gmsg);
    }

    current->flags = flags | SDL_FULLSCREEN | SDL_HWSURFACE | SDL_HWACCEL | SDL_PREALLOC;
    current->w = width;
    current->h = height;
    current->pitch = ((width * pf.BytesPerPixel) + 3) & ~3;
    if(!SDL_ReallocFormat(current, pf.BitsPerPixel, pf.Rmask, pf.Gmask, pf.Bmask, pf.Amask))
    {
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
    }

    return current;
}

static int GK_AllocHWSurface(_THIS, SDL_Surface *surface)
{
    void *smem;

    surface->pitch = ((surface->w * surface->format->BytesPerPixel) + 3) & ~3;
    smem = mmap(NULL, surface->h * surface->pitch, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON | MAP_SYNC, 0, 0);
    if(smem == MAP_FAILED)
    {
        surface->flags &= ~(SDL_HWSURFACE | SDL_HWACCEL);
        return -1;
    }
    surface->pixels = smem;
    surface->flags |= (SDL_HWSURFACE | SDL_PREALLOC | SDL_HWACCEL);

    surface->hwdata = SDL_malloc(sizeof(struct GKSurfaceData));
    if(surface->hwdata == NULL)
    {
        munmap(smem, surface->h * surface->pitch);
        memset(surface->hwdata, 0, sizeof(struct GKSurfaceData));
        ((struct GKSurfaceData *)surface->hwdata)->in_cache = 1;
    }

    return 0;
}

static void GK_FreeHWSurface(_THIS, SDL_Surface *surface)
{
    if(surface->pixels)
    {
        munmap(surface->pixels, surface->h * surface->pitch);
    }
    if(surface->hwdata)
    {
        SDL_free(surface->hwdata);
    }
}

static int GK_LockHWSurface(_THIS, SDL_Surface *surface)
{
    if(surface->flags & SDL_HWSURFACE && surface->hwdata && !surface->hwdata->in_cache)
    {
        struct gpu_message gmsgs[2];
        uint32_t spf = pformat_to_gkpf(surface->format);

        // may have been written to by DMA - need to wait and invalidate
        gmsgs[0].type = InvalidateCache;
        gmsgs[0].dest_addr = (uint32_t)(uintptr_t)surface->pixels;
        gmsgs[0].dx = 0;
        gmsgs[0].dy = 0;
        gmsgs[0].dw = surface->w;
        gmsgs[0].dh = surface->h;
        gmsgs[0].dp = surface->pitch;
        gmsgs[0].dest_pf = spf;
        
        gmsgs[1].type = SignalThread;

        GK_GPUEnqueueMessages(gmsgs, 2);

        surface->hwdata->in_cache = 1;
    }
    return 0;
}

static void GK_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
    if(!(surface->flags & SDL_HWSURFACE))
    {
        // need to clean so can be read by ltdc
        struct gpu_message gmsgs[2];
        uint32_t spf = pformat_to_gkpf(surface->format);

        // may have been written to by DMA - need to wait and invalidate
        gmsgs[0].type = CleanCache;
        gmsgs[0].dest_addr = (uint32_t)(uintptr_t)surface->pixels;
        gmsgs[0].dx = 0;
        gmsgs[0].dy = 0;
        gmsgs[0].dw = surface->w;
        gmsgs[0].dh = surface->h;
        gmsgs[0].dp = surface->pitch;
        gmsgs[0].dest_pf = spf;
        
        gmsgs[1].type = SignalThread;

        GK_GPUEnqueueMessages(gmsgs, 2);
    }
    return;
}

static int GK_FlipHWSurface(_THIS, SDL_Surface *surface)
{
    GK_GPU_CommandList(gmsg, 2);
    GK_GPUFlipBuffers(&gmsg, &surface->pixels);
    GK_GPUBlitScreenNoBlendEx(&gmsg, NULL, 0, 0, surface->w, surface->h, 0, 0);
    GK_GPUFlush(&gmsg);
    surface->flags |= SDL_PREALLOC;

    if(surface->hwdata)
    {
        surface->hwdata->in_cache = 0;
    }

    return 0;
}

static void GK_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
    SDL_Rect act_rect;

    if(numrects <= 0)
    {
        act_rect.x = 0;
        act_rect.y = 0;
        act_rect.w = this->info.current_w;
        act_rect.h = this->info.current_h;
    }
    else if(numrects == 1)
    {
        act_rect.x = rects[0].x;
        act_rect.y = rects[0].y;
        act_rect.w = rects[0].w;
        act_rect.h = rects[0].h;
    }
    else
    {
        int l = this->screen->w;
        int r = 0;
        int t = this->screen->h;
        int b = 0;

        for(int i = 0; i < numrects; i++)
        {
            if(rects[i].x < l) l = rects[i].x;
            if(rects[i].y < t) t = rects[i].y;
            if((rects[i].x + rects[i].w) > r) r = rects[i].x + rects[i].w;
            if((rects[i].y + rects[i].h) > b) b = rects[i].y + rects[i].h;
        }
        act_rect.x = l;
        act_rect.y = t;
        act_rect.w = r - l;
        act_rect.h = b - t;
    }

    GK_GPU_CommandList(gmsg, 2);
    GK_GPUFlipBuffers(&gmsg, &this->screen->pixels);
    GK_GPUBlitScreenNoBlendEx(&gmsg, NULL, act_rect.x, act_rect.y, act_rect.w, act_rect.h, 0, 0);
    GK_GPUFlush(&gmsg);
    this->screen->flags |= SDL_PREALLOC;
    if(this->screen->hwdata)
    {
        this->screen->hwdata->in_cache = 0;
    }
}

int GK_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
    return -1;
}

void GK_VideoQuit(_THIS)
{
    return;
}

static int GK_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
                                SDL_Surface *dst, SDL_Rect *dstrect)
{
    struct gpu_message gmsgs[3];
    unsigned int i = 0;

    uint32_t spf = pformat_to_gkpf(src->format);
    uint32_t dpf = pformat_to_gkpf(dst->format);
    
    if(!(src->flags & SDL_HWSURFACE))
    {
        gmsgs[i].type = CleanCache;
        gmsgs[i].dest_addr = (uint32_t)(uintptr_t)src->pixels;
        gmsgs[i].dx = srcrect->x;
        gmsgs[i].dy = srcrect->y;
        gmsgs[i].dw = srcrect->w;
        gmsgs[i].dh = srcrect->h;
        gmsgs[i].dp = src->pitch;
        gmsgs[i].dest_pf = spf;

        i++;
    }

    gmsgs[i].type = BlitImage;
    gmsgs[i].dest_addr = (uint32_t)(uintptr_t)dst->pixels;
    gmsgs[i].dest_pf = dpf;
    gmsgs[i].dx = dstrect->x;
    gmsgs[i].dy = dstrect->y;
    gmsgs[i].dw = dstrect->w;
    gmsgs[i].dh = dstrect->h;
    gmsgs[i].dp = dst->pitch;
    gmsgs[i].w = srcrect->w;
    gmsgs[i].h = srcrect->h;
    gmsgs[i].src_addr_color = (uint32_t)(uintptr_t)src->pixels;
    gmsgs[i].src_pf = spf;
    gmsgs[i].sx = srcrect->x;
    gmsgs[i].sy = srcrect->y;
    gmsgs[i].sp = src->pitch;
    
    i++;

    if(!(dst->flags & SDL_HWSURFACE))
    {
        gmsgs[i].type = SignalThread;

        i++;
    }

    GK_GPUEnqueueMessages(gmsgs, i);

    if(dst->flags & SDL_HWSURFACE && dst->hwdata)
    {
        dst->hwdata->in_cache = 0;
    }

    return 0;
}

int GK_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst)
{
    src->flags |= SDL_HWACCEL;
    src->map->hw_blit = GK_HWAccelBlit;
    return 1;
}

int GK_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color)
{
    struct gpu_message gmsg;
    uint32_t dpf = pformat_to_gkpf(dst->format);

    gmsg.dest_addr = (uint32_t)(uintptr_t)dst->pixels;
    gmsg.dx = rect->x;
    gmsg.dy = rect->y;
    gmsg.dw = rect->w;
    gmsg.dh = rect->h;
    gmsg.dp = dst->pitch;
    gmsg.src_addr_color = color;
    gmsg.dest_pf = dpf;
    gmsg.src_pf = 0;
    gmsg.type = BlitColor;
    GK_GPUEnqueueMessages(&gmsg, 1);
}

void GK_InitOSKeymap(_THIS)
{
    for(int i = 0; i < GK_NUM_SCANCODES; i++)
    {
        keymap[i] = SDLK_UNKNOWN;
    }

    for(int i = 0; i < 26; i++)
    {
        keymap[GK_SCANCODE_A + i] = SDLK_a + i;
    }
    for(int i = 0; i < 9; i++)
    {
        keymap[GK_SCANCODE_1 + i] = SDLK_1 + i;
    }
    keymap[GK_SCANCODE_0] = SDLK_0;
    for(int i = 0; i < 9; i++)
    {
        keymap[GK_SCANCODE_KP_1 + i] = SDLK_KP1 + i;
    }
    keymap[GK_SCANCODE_KP_0] = SDLK_KP0;


    keymap[GK_SCANCODE_LCTRL] = SDLK_LCTRL;
    keymap[GK_SCANCODE_RCTRL] = SDLK_RCTRL;
    keymap[GK_SCANCODE_LSHIFT] = SDLK_LSHIFT;
    keymap[GK_SCANCODE_RSHIFT] = SDLK_RSHIFT;
    keymap[GK_SCANCODE_LALT] = SDLK_LALT;
    keymap[GK_SCANCODE_RALT] = SDLK_RALT;
    keymap[GK_SCANCODE_SPACE] = SDLK_SPACE;
    keymap[GK_SCANCODE_RETURN] = SDLK_RETURN;
    keymap[GK_SCANCODE_ESCAPE] = SDLK_ESCAPE;
    keymap[GK_SCANCODE_TAB] = SDLK_TAB;

    keymap[GK_SCANCODE_RIGHT] = SDLK_RIGHT;
    keymap[GK_SCANCODE_LEFT] = SDLK_LEFT;
    keymap[GK_SCANCODE_DOWN] = SDLK_DOWN;
    keymap[GK_SCANCODE_UP] = SDLK_UP;

    for(int i = 0; i < 12; i++)
    {
        keymap[GK_SCANCODE_F1 + i] = SDLK_F1 + i;
    }

    keymap[GK_SCANCODE_PRINTSCREEN] = SDLK_PRINT;
    keymap[GK_SCANCODE_PAUSE] = SDLK_PAUSE;
    keymap[GK_SCANCODE_INSERT] = SDLK_INSERT;
    keymap[GK_SCANCODE_DELETE] = SDLK_DELETE;

    keymap[GK_SCANCODE_SLASH] = SDLK_SLASH;
    keymap[GK_SCANCODE_BACKSLASH] = SDLK_BACKSLASH;

    keymap[GK_SCANCODE_HOME] = SDLK_HOME;
    keymap[GK_SCANCODE_PAGEUP] = SDLK_PAGEUP;
    keymap[GK_SCANCODE_PAGEDOWN] = SDLK_PAGEDOWN;
    keymap[GK_SCANCODE_END] = SDLK_END;

    keymap[GK_SCANCODE_GRAVE] = SDLK_BACKQUOTE;
    keymap[GK_SCANCODE_MINUS] = SDLK_MINUS;
    keymap[GK_SCANCODE_EQUALS] = SDLK_EQUALS;
    keymap[GK_SCANCODE_LEFTBRACKET] = SDLK_LEFTBRACKET;
    keymap[GK_SCANCODE_RIGHTBRACKET] = SDLK_RIGHTBRACKET;
    keymap[GK_SCANCODE_SEMICOLON] = SDLK_SEMICOLON;
    keymap[GK_SCANCODE_APOSTROPHE] = SDLK_QUOTE;
    keymap[GK_SCANCODE_COMMA] = SDLK_COMMA;
    keymap[GK_SCANCODE_PERIOD] = SDLK_PERIOD;
    keymap[GK_SCANCODE_LGUI] = SDLK_LSUPER;
    keymap[GK_SCANCODE_APPLICATION] = SDLK_MENU;

    return;
}

void GK_PumpEvents(_THIS)
{
    struct Event ev;
    while(GK_EventPeek(&ev) > 0)
    {
        switch(ev.type)
        {
            case KeyDown:
            case KeyUp:
                {
                    SDL_keysym keysym;
                    keysym.mod = KMOD_NONE;
                    keysym.scancode = ev.key;
                    keysym.sym = ev.key < 256 ? keymap[ev.key] : ev.key;
                    SDL_PrivateKeyboard(ev.type == KeyDown ? SDL_PRESSED : SDL_RELEASED, &keysym);
                }
                break;
            case MouseMove:
                {
                    SDL_PrivateMouseMotion(ev.mouse_data.buttons, ev.mouse_data.is_rel,
                        ev.mouse_data.x, ev.mouse_data.y);                    
                }
                break;
            case MouseDown:
                for(int i = 0; i < 2; i++)
                {
                    if(ev.mouse_data.buttons & (1U << i))
                    {
                        SDL_PrivateMouseButton(SDL_PRESSED, i+1, ev.mouse_data.x, ev.mouse_data.y);
                    }
                }
                break;
            case MouseUp:
                for(int i = 0; i < 2; i++)
                {
                    if(ev.mouse_data.buttons & (1U << i))
                    {
                        SDL_PrivateMouseButton(SDL_RELEASED, i+1, ev.mouse_data.x, ev.mouse_data.y);
                    }
                }
                break;
            default:
                break;
        }
    }
}

void GK_SetCaption(_THIS, const char *caption, const char *icon_caption)
{
    GK_WindowSetTitle(caption);
}

int GK_GL_LoadLibrary(_THIS, const char *path)
{
    if(path == NULL)
        return 0;
    return -1;
}

void *GK_GL_GetProcAddress(_THIS, const char *name)
{
    return NULL;
}

int GK_GL_MakeCurrent(_THIS)
{
    if(this->gl_data == NULL)
    {
        void *firstfb;
        unsigned int gkpf;
        GLenum glpf;
        GK_GPU_CommandList(cmds, 4);

        // Get current pixel format of the display - SDL doesn't specify here
        GK_GPUGetScreenMode(NULL, NULL, &gkpf);

        // Get the first framebuffer for the requested window size
        GK_GPUSetScreenMode(&cmds, this->screen->w, this->screen->h, gkpf);
        GK_GPUClearScreen(&cmds);
        GK_GPUFlipBuffers(&cmds, &firstfb);
        GK_GPUFlush(&cmds);

        // Create a context
        switch(gkpf)
        {
            case GK_PIXELFORMAT_ARGB8888:
            case GK_PIXELFORMAT_XRGB8888:
                glpf = OSMESA_ARGB;
                break;
            case GK_PIXELFORMAT_RGB888:
                glpf = OSMESA_RGB;
                break;
            case GK_PIXELFORMAT_RGB565:
                glpf = OSMESA_RGB_565;
                break;
            default:
                SDL_SetError("Invalid pixel format");
                return -1;
        }
        this->gl_data = OSMesaCreateContext(glpf, NULL);

        if(!this->gl_data)
        {
            SDL_SetError("Couldn't allocate OSMesa context");
            return -1;
        }

        if(this->screen && this->screen->flags & SDL_NEMA)
            OSMesaEnableNema(this->gl_data, GL_TRUE);

        // Set first framebuffer
        GK_GPUGetScreenMode((size_t *)&this->screen->w, (size_t *)&this->screen->h, NULL);
        OSMesaMakeCurrent(this->gl_data, firstfb,
            this->screen->format->BitsPerPixel == 16 ? GL_UNSIGNED_SHORT_5_6_5 : GL_UNSIGNED_BYTE,
            this->screen->w, this->screen->h);
        OSMesaPixelStore(OSMESA_Y_UP, 0);
    }
    else
    {
        GK_GL_SwapWindow(this);
    }

    return 0;
}

static void GK_GL_SwapWindow(_THIS)
{
    void *next_fb;
    GK_GPU_CommandList(cmds, 4);

    if(this->screen && this->screen->flags & SDL_NEMA)
        OSMesaNemaEndFrame(this->gl_data);

    GK_GPUFlipBuffers(&cmds, &next_fb);
    GK_GPUFlush(&cmds);

    OSMesaMakeCurrent(this->gl_data, next_fb,
        this->screen->format->BitsPerPixel == 16 ? GL_UNSIGNED_SHORT_5_6_5 : GL_UNSIGNED_BYTE,
        this->screen->w, this->screen->h);
    OSMesaPixelStore(OSMESA_Y_UP, 0);
}
