#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_GK

#include "../SDL_sysrender.h"
#include "SDL_hints.h"

#include "gk.h"
#include "_gk_gpu.h"
#include <sys/mman.h>

#include <nema_core.h>

static SDL_Renderer *GKNema_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    //GK_RenderData *data;

    renderer = (SDL_Renderer *)SDL_calloc(1, sizeof(*renderer));
    if(renderer == NULL)
    {
        SDL_OutOfMemory();
        return NULL;
    }

    return NULL;        // TODO

#if 0
    data = (GK_RenderData *)SDL_calloc(1, sizeof(*data));
    if(data == NULL)
    {
        SDL_free(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

#if DEBUG_GK
    printf("GKNema_CreateRenderer\n");
#endif

    // fill in required functions
    renderer->QueueSetViewport = GK_QueueSetViewport;
    renderer->QueueSetDrawColor = GK_QueueSetDrawColor;
    renderer->QueueDrawPoints = GK_QueueDrawPoints;
    renderer->QueueDrawLines = GK_QueueDrawLines;
    renderer->QueueFillRects = GK_QueueFillRects;
    renderer->QueueCopy = GK_QueueCopy;
    renderer->QueueCopyEx = GK_QueueCopyEx;
    renderer->RunCommandQueue = GK_RunCommandQueue;
    renderer->RenderPresent = GK_RenderPresent;
    renderer->CreateTexture = GK_CreateTexture;
    renderer->UpdateTexture = GK_UpdateTexture;
    renderer->DestroyTexture = GK_DestroyTexture;
    renderer->LockTexture = GK_LockTexture;
    renderer->UnlockTexture = GK_UnlockTexture;

    renderer->info = GK_RenderDriver.info;
    renderer->driverdata = data;
    renderer->window = window;
#endif

    return renderer;
}


SDL_RenderDriver GKNema_RenderDriver = {
    .CreateRenderer = GKNema_CreateRenderer,
    .info = {
        .name = "GK renderer",
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
