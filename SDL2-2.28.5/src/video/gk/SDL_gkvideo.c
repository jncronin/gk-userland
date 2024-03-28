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
    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if(SDL_AddBasicVideoDisplay(&mode) < 0)
    {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    return 0;
}

static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    return 0;
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

    _pixels = mmap(NULL, w*h*4, PROT_READ | PROT_WRITE, MAP_ANON, 0, 0);
    if(!_pixels)
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
    struct gpu_message gmsg;

    _pixels = SDL_GetWindowData(window, "_GK_pixels");
    if(!_pixels)
    {
        return SDL_SetError("Couldn't find framebuffer for window");
    }

    /* Send data to display */
    gmsg.type = BlitImage;
    gmsg.dest_addr = 0;
    gmsg.dest_fbuf_relative = 1;
    gmsg.src_addr_color = (uint32_t)(uintptr_t)_pixels;
    gmsg.nlines = 480;
    gmsg.row_width = 640;
    gmsg.dest_pf = 0;
    gmsg.src_pf = 0;
    GK_GPUEnqueueMessages(&gmsg, 1);

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
