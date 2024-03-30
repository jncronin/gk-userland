#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_GK

#include "../SDL_sysrender.h"
#include "SDL_hints.h"

#include "gk.h"
#include "_gk_gpu.h"
#include <sys/mman.h>

#define cached_messages 64

#define GK_DESTADDR(x,y) ((data->startx + (x)) * 4 + (data->starty + (y)) * 4 * 640)
#define GK_COLOR(c) ((((uint32_t)((c).r)) << 0) | \
                        (((uint32_t)((c).g)) << 8) | \
                        (((uint32_t)((c).b)) << 16) | \
                        (((uint32_t)((c).a)) << 24))

typedef struct GK_RenderData_t
{
    uint32_t draw_color;
    struct gpu_message msgs[cached_messages];
    size_t nmsgs;
    unsigned int startx, starty;
} GK_RenderData;

/*static void send_msgs(GK_RenderData *data)
{
    GK_GPUEnqueueMessages(data->msgs, data->nmsgs);
    data->nmsgs = 0;
}*/

/*static void queue_msg(GK_RenderData *data, struct gpu_message *msg)
{
    // batch up as many messages as possible prior to sending
    memcpy(&data->msgs[data->nmsgs++], msg, sizeof(struct gpu_message));
    if(data->nmsgs >= cached_messages)
        send_msgs(data);
}*/

static int GK_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int GK_QueueSetDrawColor(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int GK_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    SDL_Point *verts = (SDL_Point *)SDL_AllocateRenderVertices(renderer, count * sizeof(SDL_Point), 0, &cmd->data.draw.first);
    int i;

    if (verts == NULL) {
        return -1;
    }

    cmd->data.draw.count = count;

    for (i = 0; i < count; i++, verts++, points++) {
        verts->x = (int)points->x;
        verts->y = (int)points->y;
    }

    return 0;
}

static int GK_QueueDrawLines(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    SDL_Point *verts = (SDL_Point *)SDL_AllocateRenderVertices(renderer, count * sizeof(SDL_Point), 0, &cmd->data.draw.first);
    int i;

    if (verts == NULL) {
        return -1;
    }

    cmd->data.draw.count = count;

    for (i = 0; i < count; i++, verts++, points++) {
        verts->x = (int)points->x;
        verts->y = (int)points->y;
    }

    return 0;
}

static int GK_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    SDL_Rect *verts = (SDL_Rect *)SDL_AllocateRenderVertices(renderer, count * sizeof(SDL_Rect), 0, &cmd->data.draw.first);
    int i;

    if (verts == NULL) {
        return -1;
    }

    cmd->data.draw.count = count;

    for (i = 0; i < count; i++, verts++, rects++) {
        verts->x = (int)rects->x;
        verts->y = (int)rects->y;
        verts->w = SDL_max((int)rects->w, 1);
        verts->h = SDL_max((int)rects->h, 1);
    }

    return 0;
}

static int GK_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                     const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    SDL_Rect *verts = (SDL_Rect *)SDL_AllocateRenderVertices(renderer, 2 * sizeof(SDL_Rect), 0, &cmd->data.draw.first);

    if (verts == NULL) {
        return -1;
    }

    cmd->data.draw.count = 1;

    SDL_copyp(verts, srcrect);
    verts++;

    verts->x = (int)dstrect->x;
    verts->y = (int)dstrect->y;
    verts->w = (int)dstrect->w;
    verts->h = (int)dstrect->h;

    return 0;
}

static int GK_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    GK_RenderData *data = (GK_RenderData *)renderer->driverdata;
    printf("GK_RunCommandQueue\n");
    while(cmd)
    {
        printf("GK: CMD: %u\n", cmd->command);
        switch(cmd->command)
        {
            case SDL_RENDERCMD_SETVIEWPORT:
                {
                    printf("GK: SDL_RENDERCMD_SETVIEWPORT: %d, %d\n",
                        cmd->data.viewport.rect.x,
                        cmd->data.viewport.rect.y);
                    data->startx = cmd->data.viewport.rect.x;
                    data->starty = cmd->data.viewport.rect.y;
                }
                break;

            case SDL_RENDERCMD_SETCLIPRECT:
                {
                    printf("GK: SDL_RENDERCMD_SETCLIPRECT: %d, %d\n",
                        cmd->data.cliprect.rect.x,
                        cmd->data.cliprect.rect.y);
                    // TODO
                }
                break;

            case SDL_RENDERCMD_SETDRAWCOLOR:
                {
                    data->draw_color = GK_COLOR(cmd->data.color);
                }
                break;

            case SDL_RENDERCMD_CLEAR:
                {
                    struct gpu_message gmsg;
                    gmsg.dest_addr = 0;
                    gmsg.dest_fbuf_relative = 1;
                    gmsg.dest_pf = 0;
                    gmsg.row_width = 640;
                    gmsg.nlines = 480;
                    gmsg.src_addr_color = data->draw_color;
                    gmsg.src_pf = 0;
                    gmsg.type = BlitColor;
                    //queue_msg(data, &gmsg);
                    GK_GPUEnqueueMessages(&gmsg, 1);
                }
                break;

            case SDL_RENDERCMD_DRAW_POINTS:
                {
                    // TODO
                }
                break;

            case SDL_RENDERCMD_DRAW_LINES:
                {
                    // TODO
                }
                break;

            case SDL_RENDERCMD_FILL_RECTS:
                {
                    const uint32_t col = GK_COLOR(cmd->data.draw);
                    const int count = (int)cmd->data.draw.count;
                    SDL_Rect *verts = (SDL_Rect *)(((Uint8 *)vertices) + cmd->data.draw.first);

                    for(int i = 0; i < count; i++)
                    {
                        struct gpu_message gmsg;

                        gmsg.dest_addr = GK_DESTADDR(verts[i].x, verts[i].y);
                        gmsg.dest_fbuf_relative = 1;
                        gmsg.dest_pf = 0;
                        gmsg.nlines = verts[i].h;
                        gmsg.row_width = verts[i].w;
                        gmsg.src_addr_color = col;
                        gmsg.src_pf = 0;
                        gmsg.type = BlitColor;
                        //queue_msg(data, &gmsg);
                        GK_GPUEnqueueMessages(&gmsg, 1);
                    }
                }
                break;

            case SDL_RENDERCMD_COPY:
                {
                    struct gpu_message gmsg;
                    SDL_Rect *verts = (SDL_Rect *) (((Uint8 *) vertices) + cmd->data.draw.first);
                    const SDL_Rect *srcrect = verts;
                    SDL_Rect *dstrect = verts + 1;
                    SDL_Texture *texture = cmd->data.draw.texture;
                    char *src = texture->driverdata;
                    int src_x = 0, src_y = 0, dest_x = 0, dest_y = 0, rw = 640, rh = 480;
                    
                    if(srcrect)
                    {
                        src_x = srcrect->x;
                        src_y = srcrect->y;
                        rw = srcrect->w;
                        rh = srcrect->h;
                    }
                    if(dstrect)
                    {
                        dest_x = dstrect->x;
                        dest_y = dstrect->y;
                    }

                    printf("RENDERCMD_COPY: src->x %d, src->y %d, dstrect: %x, dest->x %d, dest->y %d, texture->w %d, texture->h %d, src %x\n",
                        src_x, src_y, (unsigned int)(uintptr_t)dstrect, dest_x, dest_y, rw, rh, (unsigned int)(uintptr_t)src);

                    src += src_x * 4 + src_y * 4 * texture->w;

                    gmsg.type = BlitImage;
                    gmsg.dest_addr = GK_DESTADDR(dstrect->x, dstrect->y);
                    gmsg.dest_fbuf_relative = 1;
                    gmsg.dest_pf = 0;
                    gmsg.nlines = rh;
                    gmsg.row_width = rw;
                    gmsg.src_addr_color = (uint32_t)(uintptr_t)src;
                    gmsg.src_pf = 0;
                    GK_GPUEnqueueMessages(&gmsg, 1);
                    //queue_msg(data, &gmsg);
                }
                break;

            case SDL_RENDERCMD_COPY_EX:
                {
                    // TODO
                }
                break;

            case SDL_RENDERCMD_GEOMETRY:
                {
                    // TODO
                }
                break;

            case SDL_RENDERCMD_NO_OP:
                break;
        }

        cmd = cmd->next;
    }

    printf("End GK_RunCommandQueue\n");
    return 0;
}

static int GK_RenderPresent(SDL_Renderer *renderer)
{
    struct gpu_message gmsgs[2];
    printf("GK_RenderPresent\n");
    memset(gmsgs, 0, sizeof(gmsgs));
    gmsgs[0].type = FlipBuffers;
    gmsgs[1].type = SignalThread;
    GK_GPUEnqueueMessages(gmsgs, 2);
    //queue_msg((GK_RenderData *)renderer->driverdata, &gmsg);
    //send_msgs((GK_RenderData *)renderer->driverdata);

    return 0;
}

static int GK_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    void *tmem = mmap(NULL, texture->w * texture->h * 4, PROT_READ | PROT_WRITE, 
        MAP_PRIVATE | MAP_ANON, 0, 0);
    if(tmem == MAP_FAILED)
        return -1;
    texture->driverdata = tmem;

    return 0;
}

static int GK_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, const void *pixels, int pitch)
{
    struct gpu_message gmsgs[3];

    char *dest = texture->driverdata;
    dest = dest + rect->y * 640 * 4 +
        rect->x * 4;
    
    gmsgs[0].type = CleanCache;
    gmsgs[0].dest_fbuf_relative = 0;
    gmsgs[0].dest_addr = (uint32_t)(uintptr_t)pixels;
    gmsgs[0].row_width = rect->w;
    gmsgs[0].nlines = rect->h;

    gmsgs[1].type = BlitImage;
    gmsgs[1].dest_addr = (uint32_t)(uintptr_t)dest;
    gmsgs[1].dest_fbuf_relative = 0;
    gmsgs[1].dest_pf = 0;
    gmsgs[1].row_width = rect->w;
    gmsgs[1].nlines = rect->h;
    gmsgs[1].src_addr_color = (uint32_t)(uintptr_t)pixels;
    gmsgs[1].src_pf = 0;
    
    gmsgs[2].type = SignalThread;

    GK_GPUEnqueueMessages(gmsgs, 3);

    return 0;
}

static SDL_Renderer *GK_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    GK_RenderData *data;

    renderer = (SDL_Renderer *)SDL_calloc(1, sizeof(*renderer));
    if(renderer == NULL)
    {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (GK_RenderData *)SDL_calloc(1, sizeof(*data));
    if(data == NULL)
    {
        SDL_free(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    printf("GK_CreateRenderer\n");

    // fill in required functions
    renderer->QueueSetViewport = GK_QueueSetViewport;
    renderer->QueueSetDrawColor = GK_QueueSetDrawColor;
    renderer->QueueDrawPoints = GK_QueueDrawPoints;
    renderer->QueueDrawLines = GK_QueueDrawLines;
    renderer->QueueFillRects = GK_QueueFillRects;
    renderer->QueueCopy = GK_QueueCopy;
    renderer->RunCommandQueue = GK_RunCommandQueue;
    renderer->RenderPresent = GK_RenderPresent;
    renderer->CreateTexture = GK_CreateTexture;
    renderer->UpdateTexture = GK_UpdateTexture;

    renderer->info = GK_RenderDriver.info;
    renderer->driverdata = data;
    renderer->window = window;

    return renderer;
}

SDL_RenderDriver GK_RenderDriver = {
    .CreateRenderer = GK_CreateRenderer,
    .info = {
        .name = "GK renderer",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .num_texture_formats = 1,   // TODO: can we use more?
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_ARGB8888,
        },
        .max_texture_width = 640,
        .max_texture_height = 480
    }
};

#endif
