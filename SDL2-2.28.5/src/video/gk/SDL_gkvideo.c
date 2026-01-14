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
#include "../../events/SDL_mouse_c.h"

static int GK_VideoInit(_THIS);
static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode);
static void GK_VideoQuit(_THIS);
static int GK_CreateWindow(_THIS, SDL_Window *window);
static void GK_DestroyWindow(_THIS, SDL_Window *window);
static void GK_SetWindowTitle(_THIS, SDL_Window *window);
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

void OSMesaNemaEndFrame(OSMesaContext ctx);
void OSMesaEnableNema(OSMesaContext ctx, GLboolean enable);

typedef struct
{
    SDL_Window *sdl_window;
    OSMesaContext gl_ctx;
    GLenum gl_dformat;
} GK_Window;

typedef struct
{
    unsigned int gkpf;
} GK_ModeData;


uint32_t gkpf_to_pformat(unsigned int gkpf)
{
    switch(gkpf)
    {
        case GK_PIXELFORMAT_ARGB8888:
            return SDL_PIXELFORMAT_XRGB8888;
        case GK_PIXELFORMAT_RGB888:
            return SDL_PIXELFORMAT_RGB24;
        case GK_PIXELFORMAT_RGB565:
            return SDL_PIXELFORMAT_RGB565;
        case GK_PIXELFORMAT_ARGB4444:
            return SDL_PIXELFORMAT_ARGB4444;
        case GK_PIXELFORMAT_ARGB1555:
            return SDL_PIXELFORMAT_ARGB1555;
        case GK_PIXELFORMAT_L8:
            return SDL_PIXELFORMAT_INDEX8;

#if __GAMEKID__ == 4
        case GK_PIXELFORMAT_ABGR8888:
            return SDL_PIXELFORMAT_XBGR8888;
        case GK_PIXELFORMAT_RGBA8888:
            return SDL_PIXELFORMAT_RGBX8888;
        case GK_PIXELFORMAT_BGRA8888:
            return SDL_PIXELFORMAT_BGRX8888;
        case GK_PIXELFORMAT_BGR565:
            return SDL_PIXELFORMAT_BGR565;
        case GK_PIXELFORMAT_A4L4:
            return SDL_PIXELFORMAT_INDEX4LSB;
#endif

        default:
            return 0;
    }
}

unsigned int pformat_to_gkpf(uint32_t sdlpf)
{
    switch(sdlpf)
    {
        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_XRGB8888:
            return GK_PIXELFORMAT_ARGB8888;
        case SDL_PIXELFORMAT_RGB24:
            return GK_PIXELFORMAT_RGB888;
        case SDL_PIXELFORMAT_RGB565:
            return GK_PIXELFORMAT_RGB565;
        case SDL_PIXELFORMAT_ARGB4444:
        case SDL_PIXELFORMAT_XRGB4444:
            return GK_PIXELFORMAT_ARGB4444;
        case SDL_PIXELFORMAT_ARGB1555:
        case SDL_PIXELFORMAT_XRGB1555:
            return GK_PIXELFORMAT_ARGB1555;
        case SDL_PIXELFORMAT_INDEX8:
            return GK_PIXELFORMAT_L8;

#if __GAMEKID__ == 4
        case SDL_PIXELFORMAT_ABGR8888:
        case SDL_PIXELFORMAT_XBGR8888:
            return GK_PIXELFORMAT_ABGR8888;
        case SDL_PIXELFORMAT_RGBA8888:
        case SDL_PIXELFORMAT_RGBX8888:
            return GK_PIXELFORMAT_RGBA8888;
        case SDL_PIXELFORMAT_BGRA8888:
        case SDL_PIXELFORMAT_BGRX8888:
            return GK_PIXELFORMAT_BGRA8888;
        case SDL_PIXELFORMAT_BGR565:
            return GK_PIXELFORMAT_BGR565;
        case SDL_PIXELFORMAT_INDEX4LSB:
            return GK_PIXELFORMAT_A4L4;
#endif

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
    device->SetWindowTitle = GK_SetWindowTitle;

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

static int GKV4_AddModes(_THIS)
{
    static const unsigned int refresh_rates[] = { 60, 50, 48, 40, 30, 25, 24 };
    static const unsigned int pformats[] =
    {
        SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_RGB888,
        SDL_PIXELFORMAT_RGB24,
        SDL_PIXELFORMAT_RGB565,
        SDL_PIXELFORMAT_ARGB4444,
        SDL_PIXELFORMAT_ARGB1555,
        SDL_PIXELFORMAT_INDEX8,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_PIXELFORMAT_BGR565,
        SDL_PIXELFORMAT_INDEX4LSB
    };
    struct wh
    {
        unsigned int w;
        unsigned int h;
    };
    static const struct wh whs[] =
    {
        { .w = 1024, .h = 768 },
        { .w = 720, .h = 576 },
        { .w = 800, .h = 480 },
        { .w = 640, .h = 480 },
        { .w = 400, .h = 240 },
        { .w = 320, .h = 240 },
        { .w = 200, .h = 120 },
        { .w = 160, .h = 120 }
    };

    for(unsigned int wh_idx = 0; wh_idx < sizeof(whs) / sizeof(whs[0]); wh_idx++)
    {
        for(unsigned int pf_idx = 0; pf_idx < sizeof(pformats) / sizeof(pformats[0]); pf_idx++)
        {
            for(unsigned int rr_idx = 0; rr_idx < sizeof(refresh_rates) / sizeof(refresh_rates[0]); rr_idx++)
            {
                SDL_DisplayMode mode;
                unsigned int w = whs[wh_idx].w;
                unsigned int h = whs[wh_idx].h;
                unsigned int pf = pformats[pf_idx];
                unsigned int rr = refresh_rates[rr_idx];

                if(w > GK_KERNEL_INFO->max_screen_width)
                {
                    continue;
                }
                if(h > GK_KERNEL_INFO->max_screen_height)
                {
                    continue;
                }

                mode.w = w;
                mode.h = h;
                mode.refresh_rate = rr;
                mode.format = pf;
                mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
                if(!mode.driverdata)
                {
                    return -1;
                }
                ((GK_ModeData *)mode.driverdata)->gkpf = pformat_to_gkpf(pf);
                SDL_AddDisplayMode(&_this->displays[0], &mode);
            }
        }
    }
    return 0;
}

int GK_VideoInit(_THIS)
{
    SDL_DisplayMode mode;
    unsigned int gkpf;
#if __GAMEKID__ != 4
    static const unsigned int refresh_rates[] = { 60, 50, 48, 40, 30, 25, 24 };
#endif

    /* Get current mode */
    GK_GPUGetScreenModeEx((size_t *)&mode.w, (size_t *)&mode.h, &gkpf, &mode.refresh_rate);
    mode.format = gkpf_to_pformat(gkpf);
    mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
    if(!mode.driverdata)
    {
        return -1;
    }
    ((GK_ModeData *)mode.driverdata)->gkpf = gkpf;
    if(SDL_AddBasicVideoDisplay(&mode) < 0)
    {
        return -1;
    }

#if __GAMEKID__ == 4
    GKV4_AddModes(_this);
#else
    /* Allow 640x480, 320x240, 160x120 at 32-/24-/16-/8-bpp */
    for(int rr_idx = 0; rr_idx < (sizeof(refresh_rates) / sizeof(refresh_rates[0])); rr_idx++)
    {
        for(int sdiv = 1; sdiv <= 3; sdiv++)
        {
            mode.w = 640 / sdiv;
            mode.h = 480 / sdiv;
            mode.refresh_rate = refresh_rates[rr_idx];

            mode.format = SDL_PIXELFORMAT_ARGB8888;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_ARGB8888;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);
            if(sdiv == 1)
            {
                if(SDL_AddBasicVideoDisplay(&mode) < 0)
                {
                    return -1;
                }
            }
            else
            {
            }

            mode.format = SDL_PIXELFORMAT_RGB24;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_RGB888;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);

            mode.format = SDL_PIXELFORMAT_RGB565;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_RGB565;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);

            mode.format = SDL_PIXELFORMAT_ARGB4444;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_ARGB4444;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);

            mode.format = SDL_PIXELFORMAT_ARGB1555;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_ARGB1555;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);

            mode.format = SDL_PIXELFORMAT_INDEX8;
            mode.driverdata = SDL_malloc(sizeof(GK_ModeData));
            if(!mode.driverdata)
            {
                return -1;
            }
            ((GK_ModeData *)mode.driverdata)->gkpf = GK_PIXELFORMAT_L8;        
            SDL_AddDisplayMode(&_this->displays[0], &mode);
        }
    }
#endif

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    return 0;
}

static int GK_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    unsigned int pf;int supported = 1;
    if(!mode)
    {
        return -1;
    }
    if(mode->driverdata)
    {
        pf = ((GK_ModeData *)mode->driverdata)->gkpf;
    }
    else
    {
#if __GAMEKID__ == 4
        pf = pformat_to_gkpf(mode->format);
#else
        return -1;
#endif
    }


    switch(pf)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            break;
        
        default:
#if __GAMEKID__ == 4
            if(pf > GK_PIXELFORMAT_MAX)
#endif
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
#if __GAMEKID__ == 4
        if((mode->w >= GK_KERNEL_INFO->max_screen_width) ||
            (mode->w & 0x3) ||
            (mode->h >= GK_KERNEL_INFO->max_screen_height) ||
            (mode->h & 0x3))
#endif
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
            g.sx = mode->refresh_rate;
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
    SDL_SetKeyboardFocus(window);

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

void GK_SetWindowTitle(_THIS, SDL_Window *window)
{
    GK_WindowSetTitle(window->title);
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
    SDL_Rect act_rect;
    GK_GPU_CommandList(gmsg, 2);

    if(numrects <= 0)
    {
        act_rect.x = 0;
        act_rect.y = 0;
        act_rect.w = _this->displays->current_mode.w;
        act_rect.h = _this->displays->current_mode.h;
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
        int l = _this->displays->current_mode.w;
        int r = 0;
        int t = _this->displays->current_mode.h;
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

    GK_GPUFlipBuffers(&gmsg, &window->surface->pixels);
    GK_GPUBlitScreenNoBlendEx(&gmsg, NULL, act_rect.x, act_rect.y, act_rect.w, act_rect.h, 0, 0);
    gmsg.msgs[gmsg.hdr.__ncmds-1].type = BlitImageNoBlendIf;    /* will not copy if flagged as such in process struct */
    GK_GPUFlush(&gmsg);
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
            case MouseMove:
                SDL_SendMouseMotion(SDL_GetKeyboardFocus(), 0, ev.mouse_data.is_rel,
                    ev.mouse_data.x, ev.mouse_data.y);
                break;
            case MouseUp:
                if(ev.mouse_data.is_rel)
                {
                    SDL_SendMouseMotion(SDL_GetKeyboardFocus(), 0, ev.mouse_data.is_rel,
                        ev.mouse_data.x, ev.mouse_data.y);
                }
                SDL_SendMouseButton(SDL_GetKeyboardFocus(), 0, SDL_RELEASED,
                    ev.mouse_data.buttons == 1 ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT);
                break;
            case MouseDown:
                if(ev.mouse_data.is_rel)
                {
                    SDL_SendMouseMotion(SDL_GetKeyboardFocus(), 0, ev.mouse_data.is_rel,
                        ev.mouse_data.x, ev.mouse_data.y);
                }
                SDL_SendMouseButton(SDL_GetKeyboardFocus(), 0, SDL_PRESSED,
                    ev.mouse_data.buttons == 1 ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT);
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

    if(window->flags & SDL_WINDOW_NEMA)
        OSMesaEnableNema(gk_window->gl_ctx, GL_TRUE);

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

    if(window->flags & SDL_WINDOW_NEMA)
        OSMesaNemaEndFrame(gk_window->gl_ctx);

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
