/* GK video driver for SDL2 */
#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_GK

#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include <sys/mman.h>
#include <gk.h>
#include <GL/gl.h>
#include <GL/osmesa.h>

#include "../../events/SDL_keyboard_c.h"

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
static int GK_GL_LoadLibrary(_THIS, const char *path);
static void *GK_GL_GetProcAddress(_THIS, const char *name);

typedef struct
{
    SDL_Window *sdl_window;
    OSMesaContext gl_ctx;
    GLenum gl_dformat;
} GK_Window;

static uint32_t gkpf_to_pformat(unsigned int gkpf)
{
    switch(gkpf)
    {
        case GK_PIXELFORMAT_ARGB8888:
            return SDL_PIXELFORMAT_XRGB8888;
        case GK_PIXELFORMAT_RGB888:
            return SDL_PIXELFORMAT_RGB888;
        case GK_PIXELFORMAT_RGB565:
            return SDL_PIXELFORMAT_RGB565;
        case GK_PIXELFORMAT_L8:
            return SDL_PIXELFORMAT_INDEX8;
        default:
            return 0;
    }
}

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

    device->GL_LoadLibrary = GK_GL_LoadLibrary;
    device->GL_GetProcAddress = GK_GL_GetProcAddress;
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
    size_t w, h;
    unsigned int gkpf;
    GK_GPU_CommandList(gmsg, 1);

    GK_GPUGetScreenMode(&w, &h, &gkpf);
    *format = gkpf_to_pformat(gkpf);
    *pitch = w * SDL_BYTESPERPIXEL(*format);

    GK_GPUFlipBuffers(&gmsg, pixels);
    GK_GPUFlush(&gmsg);

    if(window->surface)
    {
        window->surface->w = w;
        window->surface->h = h;
        window->surface->pitch = *pitch;
        window->surface->pixels = *pixels;
    }

    return 0;
}

int GK_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
{
    /* We are allowed to update the window's surface's pixels member here */
    const unsigned int nmsgs = 8;
    unsigned int msg_in_block = 0;
    struct gpu_message gmsg[nmsgs];

    for(int cur_msg = 0; cur_msg < (numrects + 2); cur_msg++)
    {
        struct gpu_message *cgmsg = &gmsg[msg_in_block++];

        if(cur_msg == 0)
        {
            /* First message is to flip buffers */
            cgmsg->type = FlipBuffers;
            cgmsg->dest_addr = (uint32_t)(uintptr_t)&window->surface->pixels;
            cgmsg->src_addr_color = 0;
        }
        else if(cur_msg < (numrects + 1))
        {
            const SDL_Rect *crect = &rects[cur_msg - 1];

            /* Next 'n' messages are update rects, from front to backbuffer */
            cgmsg->type = BlitImage;
            cgmsg->dest_addr = 0;
            cgmsg->dest_pf = 0;
            cgmsg->dp = 0;
            cgmsg->dx = crect->x;
            cgmsg->dy = crect->y;
            cgmsg->dw = crect->w;
            cgmsg->dh = crect->h;
            cgmsg->src_addr_color = 0;
            cgmsg->src_pf = 0;
            cgmsg->sp = 0;
            cgmsg->sx = crect->x;
            cgmsg->sy = crect->y;
            cgmsg->w = crect->w;
            cgmsg->h = crect->h;
        }
        else
        {
            /* Final message is to wait for completion, so we can continue updating the
                surface on the next frame */
            cgmsg->type = SignalThread;
            cgmsg->dest_addr = 0;
            cgmsg->src_addr_color = 0;
        }

        if(msg_in_block == nmsgs)
        {
            /* Need to send this batch of messages */
            GK_GPUEnqueueMessages(gmsg, msg_in_block);
            msg_in_block = 0;
        }
    }

    /* Send any messages remaining */
    GK_GPUEnqueueMessages(gmsg, msg_in_block);

    return 0;
}

void GK_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    /* Nothing here - we didn't allocate the pixels for the framebuffer */
}

void GK_PumpEvents(_THIS)
{
    struct Event ev;
    while(GK_EventPeek(&ev) > 0)
    {
        switch(ev.type)
        {
            case KeyDown:
                SDL_SendKeyboardKey(SDL_PRESSED, (SDL_Scancode)ev.key);
                break;
            case KeyUp:
                SDL_SendKeyboardKey(SDL_RELEASED, (SDL_Scancode)ev.key);
                break;
            default:
                break;
        }
    }
}

SDL_GLContext GK_GL_CreateContext(_THIS, SDL_Window *window)
{
    void *firstfb;
    GK_Window *gk_window = window->driverdata;
    unsigned int gkpf;
    GLenum glpf;
    GK_GPU_CommandList(cmds, 4);

    // Get current pixel format of the display - SDL doesn't specify here
    GK_GPUGetScreenMode(NULL, NULL, &gkpf);

    // Get the first framebuffer for the requested window size
    GK_GPUSetScreenMode(&cmds, window->w, window->h, gkpf);
    GK_GPUClearScreen(&cmds);
    GK_GPUFlipBuffers(&cmds, &firstfb);
    GK_GPUFlush(&cmds);

    // Create a context
    switch(gkpf)
    {
        case GK_PIXELFORMAT_ARGB8888:
        case GK_PIXELFORMAT_XRGB8888:
            glpf = OSMESA_ARGB;
            gk_window->gl_dformat = GL_UNSIGNED_BYTE;
            break;
        case GK_PIXELFORMAT_RGB888:
            glpf = OSMESA_RGB;
            gk_window->gl_dformat = GL_UNSIGNED_BYTE;
            break;
        case GK_PIXELFORMAT_RGB565:
            glpf = OSMESA_RGB_565;
            gk_window->gl_dformat = GL_UNSIGNED_SHORT_5_6_5;
            break;
        default:
            return NULL;
    }
    gk_window->gl_ctx = OSMesaCreateContext(glpf, NULL);

    // Set first framebuffer
    GK_GPUGetScreenMode((size_t *)&window->w, (size_t *)&window->h, NULL);
    OSMesaMakeCurrent(gk_window->gl_ctx, firstfb, gk_window->gl_dformat, window->w, window->h);
    OSMesaPixelStore(OSMESA_Y_UP, 0);

    return gk_window->gl_ctx;
}

int GK_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    GK_Window *gk_window = window->driverdata;
    gk_window->gl_ctx = (OSMesaContext)context;
    return 0;
}

int GK_GL_GetSwapInterval(_THIS)
{
    return 1;       // always vsync
}

int GK_GL_SwapWindow(_THIS, SDL_Window *window)
{
    void *next_fb;
    GK_Window *gk_window = window->driverdata;
    GK_GPU_CommandList(cmds, 4);

    GK_GPUFlipBuffers(&cmds, &next_fb);
    GK_GPUFlush(&cmds);

    OSMesaMakeCurrent(gk_window->gl_ctx, next_fb, gk_window->gl_dformat,   
        window->w, window->h);
    OSMesaPixelStore(OSMESA_Y_UP, 0);

    return 0;    
}

void GK_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    OSMesaDestroyContext((OSMesaContext)context);
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

#endif
