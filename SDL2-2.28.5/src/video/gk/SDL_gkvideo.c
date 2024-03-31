/* GK video driver for SDL2 */
#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_GK

#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include <sys/mman.h>
#include <gk.h>


static int GK_VideoInit(_THIS);
static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode);
static void GK_VideoQuit(_THIS);
static int GK_CreateWindow(_THIS, SDL_Window *window);
static void GK_DestroyWindow(_THIS, SDL_Window *window);
static int GK_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format,
    void **pixels, int *pitch);
static int GK_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects);
static void GK_DestroyWindowFramebuffer(_THIS, SDL_Window *window);
static void GK_PumpEvents(_THIS);

typedef struct
{
    SDL_Window *sdl_window;
} GK_Window;


static void GK_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *GK_CreateDevice(void)
{
    SDL_VideoDevice *device;
    device = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
    if(device == NULL)
    {
        SDL_OutOfMemory();
        return 0;
    }

    device->VideoInit = GK_VideoInit;
    device->VideoQuit = GK_VideoQuit;
    device->SetDisplayMode = GK_SetDisplayMode;
    device->PumpEvents = GK_PumpEvents;
    device->CreateWindowFramebuffer = GK_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = GK_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = GK_DestroyWindowFramebuffer;
    device->free = GK_DeleteDevice;
    device->CreateSDLWindow = GK_CreateWindow;
    device->DestroyWindow = GK_DestroyWindow;

    return device;
}

VideoBootStrap GK_bootstrap = {
    "gk", "gk video driver", GK_CreateDevice
};

int GK_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 640;
    mode.h = 480;
    mode.refresh_rate = 60;
    mode.driverdata = (void *)0;
    if(SDL_AddBasicVideoDisplay(&mode) < 0)
    {
        return -1;
    }

    mode.format = SDL_PIXELFORMAT_RGB888;
    mode.driverdata = (void *)1;
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.driverdata = (void *)2;
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    mode.format = SDL_PIXELFORMAT_INDEX8;
    mode.driverdata = (void *)5;
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    return 0;
}

static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    int pf = (int)mode->driverdata;
    if(!mode)
    {
        return -1;
    }

    switch(pf)
    {
        case 0:
        case 1:
        case 2:
        case 5:
            // supported
            {
                struct gpu_message g;
                g.type = SetBuffers;
                g.dest_addr = 0;
                g.src_addr_color = 0;
                g.dest_pf = (uint32_t)pf;
                GK_GPUEnqueueMessages(&g, 1);
            }
            return 0;
    }
    return -1;
}

void GK_VideoQuit(_THIS)
{

}

int GK_CreateWindow(_THIS, SDL_Window *window)
{
    GK_Window *gk_window = SDL_calloc(1, sizeof(GK_Window));

    if(gk_window == NULL)
    {
        return SDL_OutOfMemory();
    }

    window->driverdata = gk_window;
    if(window->x == SDL_WINDOWPOS_UNDEFINED)
    {
        window->x = 0;
    }
    if(window->y == SDL_WINDOWPOS_UNDEFINED)
    {
        window->y = 0;
    }

    gk_window->sdl_window = window;

    return 0;
}

void GK_DestroyWindow(_THIS, SDL_Window *window)
{
    GK_Window *gk_window = window->driverdata;

    if(gk_window)
    {
        SDL_free(gk_window);
    }

    window->driverdata = NULL;
}

int GK_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format,
    void **pixels, int *pitch)
{
    const Uint32 surface_format = SDL_PIXELFORMAT_ARGB8888;
    int w, h;
    void *_pixels;

    GK_DestroyWindowFramebuffer(_this, window);

    SDL_GetWindowSizeInPixels(window, &w, &h);

    _pixels = mmap(NULL, w*h*4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    if(_pixels == MAP_FAILED)
    {
        return -1;
    }

    SDL_SetWindowData(window, "_GK_pixels", _pixels);
    *format = surface_format;
    *pixels = _pixels;
    *pitch = w*4;

    return 0;
}

int GK_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
{
    void *_pixels;
    struct gpu_message gmsg[4];

    _pixels = SDL_GetWindowData(window, "_GK_pixels");
    if(!_pixels)
    {
        return SDL_SetError("Couldn't find framebuffer for window");
    }
    

    /* Send data to display */
    gmsg[0].type = CleanCache;
    gmsg[0].dest_addr = (uint32_t)(uintptr_t)_pixels;
    gmsg[0].dest_pf = GK_PIXELFORMAT_ARGB8888;
    gmsg[0].dx = 0;
    gmsg[0].dy = 0;
    gmsg[0].w = window->w;
    gmsg[0].h = window->h;
    gmsg[0].dp = window->w * 4;

    gmsg[1].type = BlitImage;
    gmsg[1].dest_addr = 0;
    gmsg[1].dx = 0;
    gmsg[1].dy = 0;
    gmsg[1].src_addr_color = (uint32_t)(uintptr_t)_pixels;
    gmsg[1].w = window->w;
    gmsg[1].h = window->h;
    gmsg[1].src_pf = 0;
    gmsg[1].sp = window->w * 4;

    gmsg[2].type = FlipBuffers;

    gmsg[3].type = SignalThread;

    GK_GPUEnqueueMessages(gmsg, 4);

    return 0;
}

void GK_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    void *_pixels;
    int w, h;

    SDL_GetWindowSizeInPixels(window, &w, &h);
    _pixels = SDL_GetWindowData(window, "_GK_pixels");
    if(_pixels)
    {
        munmap(_pixels, w*h*4);
        SDL_SetWindowData(window, "_GK_pixels", NULL);
    }
}

void GK_PumpEvents(_THIS)
{

}

#endif
