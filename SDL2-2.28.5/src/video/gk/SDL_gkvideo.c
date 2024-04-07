/* GK video driver for SDL2 */
#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_GK

#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include <sys/mman.h>
#include <gk.h>
#include <zbuffer.h>
#include <GL/gl.h>

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

static SDL_GLContext GK_GL_CreateContext(_THIS, SDL_Window *window);
static int GK_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
static int GK_GL_GetSwapInterval(_THIS);
static int GK_GL_SwapWindow(_THIS, SDL_Window *window);
static void GK_GL_DeleteContext(_THIS, SDL_GLContext context);

typedef struct
{
    SDL_Window *sdl_window;
    ZBuffer *gl_fb;
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

    device->GL_LoadLibrary = NULL;
    device->GL_GetProcAddress = NULL;
    device->GL_UnloadLibrary = NULL;
    device->GL_CreateContext = GK_GL_CreateContext;
    device->GL_MakeCurrent = GK_GL_MakeCurrent;
    device->GL_GetSwapInterval = GK_GL_GetSwapInterval;
    device->GL_SwapWindow = GK_GL_SwapWindow;
    device->GL_DeleteContext = GK_GL_DeleteContext;

    return device;
}

VideoBootStrap GK_bootstrap = {
    "gk", "gk video driver", GK_CreateDevice
};

int GK_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    /* Allow 640x480, 320x240, 160x120 at 32-/24-/16-/8-bpp */

    for(int sdiv = 1; sdiv <= 3; sdiv++)
    {
        mode.format = SDL_PIXELFORMAT_ARGB8888;
        mode.w = 640 / sdiv;
        mode.h = 480 / sdiv;
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
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    return 0;
}

static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    int supported = 1;
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
            break;
        default:
            supported = 0;
            break;
    }

    if(mode->w == 640 && mode->h == 480)
    {
        // supported
    }
    else if(mode->w == 320 && mode->h == 240)
    {
        // supported
    }
    else if(mode->w == 160 && mode->h == 120)
    {
        // supported
    }
    else
    {
        supported = 0;
    }

    if(supported)
    {
        // supported
        {
            struct gpu_message g;
            g.type = SetScreenMode;
            g.w = mode->w;
            g.h = mode->h;
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
    gmsg[2].dest_addr = 0;
    gmsg[2].src_addr_color = 0;

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

SDL_GLContext GK_GL_CreateContext(_THIS, SDL_Window *window)
{
    ZBuffer *framebuffer;
    void *firstfb;
    GK_Window *gk_window = window->driverdata;
    unsigned int pf = TGL_FEATURE_RENDER_BITS == 32 ? GK_PIXELFORMAT_ARGB8888 : GK_PIXELFORMAT_RGB565;
    GK_GPU_CommandList(cmds, 4);

    // Get the first framebuffer for the requested window size
    GK_GPUSetScreenMode(&cmds, window->w, window->h, pf);
    GK_GPUClearScreen(&cmds);
    GK_GPUFlipBuffers(&cmds, &firstfb);
    GK_GPUFlush(&cmds);

    // Init a tinygl zbuffer with this
    framebuffer = ZB_open(window->w, window->h,
        ZB_MODE_5R6G5B, firstfb);
    if(framebuffer)
    {
        glInit(framebuffer);
        gk_window->gl_fb = framebuffer;
    }
    return framebuffer;
}

int GK_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    GK_Window *gk_window = window->driverdata;
    gk_window->gl_fb = (ZBuffer *)context;
    return 0;
}

int GK_GL_GetSwapInterval(_THIS)
{
    return 1;       // always vsync
}

int GK_GL_SwapWindow(_THIS, SDL_Window *window)
{
    GK_Window *gk_window = window->driverdata;
    GK_GPU_CommandList(cmds, 4);

    GK_GPUFlipBuffers(&cmds, (void **)&gk_window->gl_fb->pbuf);
    GK_GPUFlush(&cmds);
    return 0;    
}

void GK_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    ZBuffer *fb = (ZBuffer *)context;
    fb->frame_buffer_allocated = 0;
    fb->pbuf = 0;
    ZB_close(fb);
}

#endif
