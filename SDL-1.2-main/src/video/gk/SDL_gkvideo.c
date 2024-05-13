#include "SDL_config.h"
#include "SDL.h"
#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "gk.h"
#include <sys/mman.h>


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
    device->SetCaption = NULL;
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

    current->flags = flags | SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL;
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
        return -1;
    }
    surface->pixels = smem;
    surface->flags |= (SDL_HWSURFACE | SDL_PREALLOC | SDL_HWACCEL);
    return 0;
}

static void GK_FreeHWSurface(_THIS, SDL_Surface *surface)
{
    if(surface->pixels)
    {
        munmap(surface->pixels, surface->h * surface->pitch);
    }
}

static int GK_LockHWSurface(_THIS, SDL_Surface *surface)
{
    return 0;
}

static void GK_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
    return;
}

static int GK_FlipHWSurface(_THIS, SDL_Surface *surface)
{
    GK_GPU_CommandList(gmsg, 1);
    GK_GPUFlipBuffers(&gmsg, &surface->pixels);
    GK_GPUFlush(&gmsg);
    return 0;
}

static void GK_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
    return;
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

    uint32_t spf = pformat_to_gkpf(src->format);
    uint32_t dpf = pformat_to_gkpf(dst->format);
    
    gmsgs[0].type = CleanCache;
    gmsgs[0].dest_addr = (uint32_t)(uintptr_t)src->pixels;
    gmsgs[0].dx = srcrect->x;
    gmsgs[0].dy = srcrect->y;
    gmsgs[0].w = srcrect->w;
    gmsgs[0].h = srcrect->h;
    gmsgs[0].dp = src->pitch;
    gmsgs[0].dest_pf = spf;

    gmsgs[1].type = BlitImage;
    gmsgs[1].dest_addr = (uint32_t)(uintptr_t)dst->pixels;
    gmsgs[1].dest_pf = dpf;
    gmsgs[1].dx = dstrect->x;
    gmsgs[1].dy = dstrect->y;
    gmsgs[1].dw = dstrect->w;
    gmsgs[1].dh = dstrect->h;
    gmsgs[1].dp = dst->pitch;
    gmsgs[1].w = srcrect->w;
    gmsgs[1].h = srcrect->h;
    gmsgs[1].src_addr_color = (uint32_t)(uintptr_t)src->pixels;
    gmsgs[1].src_pf = spf;
    gmsgs[1].sx = srcrect->x;
    gmsgs[1].sy = srcrect->y;
    gmsgs[1].sp = src->pitch;
    
    gmsgs[2].type = SignalThread;

    GK_GPUEnqueueMessages(gmsgs, 3);

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
                    keysym.sym = ev.key;
                    SDL_PrivateKeyboard(ev.type == KeyDown ? SDL_PRESSED : SDL_RELEASED, &keysym);
                }
                break;
            default:
                break;
        }
    }
}
