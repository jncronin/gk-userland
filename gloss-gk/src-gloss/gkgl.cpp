#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "gkgl.h"
#include "gk.h"

struct gkgl_ctx
{
    unsigned int w = 0, h = 0;

    EGLContext ctx = nullptr;
    EGLDisplay d = nullptr;
    gbm_device *gbm = nullptr;

    struct gbm_bo *tiled_bo = nullptr;
    struct gbm_bo *linear_bos[3] = { nullptr, nullptr, nullptr };

    EGLImageKHR tiled_image = nullptr;
    EGLImageKHR linear_images[3] = { nullptr, nullptr, nullptr };

    GLuint linear_fbs[3] = { 0, 0, 0 };
    GLuint linear_rbs[3] = { 0, 0, 0 };
    GLuint tiled_fb = 0;
    GLuint tiled_rb = 0;
    int native_id = 0;

    /* These are just to ensure the required functions actually
        get linked so can be dlsym'd() */
    void (*force_link1)() = nullptr;
    void (*force_link2)() = nullptr;
};

extern "C" {
    void GLAPIENTRY _mesa_EGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);
    void GLAPIENTRY _mesa_EGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image);
    // ensure these functions are linked
    void __driDriverGetExtensions_etnaviv();
    void gbmint_get_backend();
}

int GKGLAttribInit(GKGLAttribs *attrs)
{
    if(!attrs)
    {
        errno = EINVAL;
        return -1;
    }

    size_t w, h;
    GK_GPUGetScreenMode(&w, &h, nullptr);
    
    attrs->width = w;
    attrs->height = h;
    attrs->rsize = 8;
    attrs->gsize = 8;
    attrs->bsize = 8;
    attrs->asize = 8;
    attrs->depth_size = 16;
    attrs->stencil_size = 8;
    attrs->maj_ver = 3;
    attrs->min_ver = 1;
    attrs->core_profile = 0;

    return 0;
}

int GKGLCreateContext(GKGLContext *_ctx, GKGLAttribs *attrs)
{
    if(!_ctx)
    {
        errno = EINVAL;
        return -1;
    }
    GKGLAttribs def_attrs;
    if(!attrs)
    {
        GKGLAttribInit(&def_attrs);
        attrs = &def_attrs;
    }
    
    auto ctx = new gkgl_ctx();
    
    int fd = open("/dev/dri/renderD128", O_RDWR); 
    ctx->gbm = gbm_create_device(fd);

    if(ctx->gbm == nullptr)
    {
        fprintf(stderr, "gbm_create_device failed: %d\n", errno);
        GKGLDeleteContext(ctx);
        return -1;
    }

    ctx->d = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, ctx->gbm, nullptr);
    if(ctx->d == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "eglGetDisplay failed\n");
        GKGLDeleteContext(ctx);
        return -1;
    }

    int egl_maj, egl_min;
    if(!eglInitialize(ctx->d, &egl_maj, &egl_min))
    {
        fprintf(stderr, "eglInitialize failed\n");
        GKGLDeleteContext(ctx);
        return -1;
    }
    fprintf(stderr, "egl ver %d.%d\n", egl_maj, egl_min);
    fprintf(stderr, "egl vendor %s\n", eglQueryString(ctx->d, EGL_VENDOR));

    eglBindAPI(EGL_OPENGL_API);

    EGLint const attrib_list[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, attrs->rsize,
        EGL_GREEN_SIZE, attrs->gsize,
        EGL_BLUE_SIZE, attrs->bsize,
        EGL_ALPHA_SIZE, attrs->asize,
        EGL_DEPTH_SIZE, attrs->depth_size,
        EGL_STENCIL_SIZE, attrs->stencil_size,
        EGL_NONE
    };

    EGLConfig configs[16];
    int nconfigs = 0;
    if(!eglChooseConfig(ctx->d, attrib_list, configs, sizeof(configs) / sizeof(configs[0]), &nconfigs))
    {
        fprintf(stderr, "eglChooseConfig failed\n");
        GKGLDeleteContext(ctx);
        return -1;
    }
    fprintf(stderr, "egl found %d configs\n", nconfigs);
    if(nconfigs == 0)
    {
        GKGLDeleteContext(ctx);
        return -1;
    }

    auto &conf = configs[0];
    eglGetConfigAttrib(ctx->d, conf, EGL_RED_SIZE, &attrs->rsize);
    eglGetConfigAttrib(ctx->d, conf, EGL_GREEN_SIZE, &attrs->gsize);
    eglGetConfigAttrib(ctx->d, conf, EGL_BLUE_SIZE, &attrs->bsize);
    eglGetConfigAttrib(ctx->d, conf, EGL_ALPHA_SIZE, &attrs->asize);
    eglGetConfigAttrib(ctx->d, conf, EGL_DEPTH_SIZE, &attrs->depth_size);
    eglGetConfigAttrib(ctx->d, conf, EGL_STENCIL_SIZE, &attrs->stencil_size);
    eglGetConfigAttrib(ctx->d, conf, EGL_NATIVE_VISUAL_ID, &ctx->native_id);

    ctx->ctx = eglCreateContext(ctx->d, conf, EGL_NO_CONTEXT, nullptr);
    if(ctx == nullptr)
    {
        fprintf(stderr, "eglCreateContext failed\n");
        GKGLDeleteContext(ctx);
        return -1;
    }

    ctx->w = attrs->width;
    ctx->h = attrs->height;

    //ctx->force_link1 = __driDriverGetExtensions_etnaviv;
    ctx->force_link2 = gbmint_get_backend;

    *_ctx = ctx;
    return 0;
}

int GKGLMakeCurrent(GKGLContext ctx)
{
    if(!ctx)
    {
        errno = EINVAL;
        return -1;
    }

    if(!eglMakeCurrent(ctx->d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx->ctx))
    {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return -1;
    }

    // tiled buffer
    ctx->tiled_bo = gbm_bo_create(ctx->gbm, ctx->w, ctx->h, ctx->native_id, 
        GBM_BO_USE_RENDERING);
    if(!ctx->tiled_bo)
    {
        fprintf(stderr, "tiled_bo null %d\n", errno);
        return -1;
    }

    // Wrap in EGLimage
    ctx->tiled_image = eglCreateImage(ctx->d, EGL_NO_CONTEXT, 
                                EGL_NATIVE_PIXMAP_KHR, ctx->tiled_bo, NULL);

    // Create a tiled FBO and attach the tiled image
    glGenFramebuffers(1, &ctx->tiled_fb);
    glGenRenderbuffers(1, &ctx->tiled_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, ctx->tiled_rb);
    _mesa_EGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, ctx->tiled_image);
    glBindFramebuffer(GL_FRAMEBUFFER, ctx->tiled_fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ctx->tiled_rb);

    // Now create three linear buffers (these are the screen framebuffers)
    for(int i = 0; i < 3; i++)
    {
        // linear screen buffer
        GK_GPUCreateRenderBufferNext(i);
        ctx->linear_bos[i] = gbm_bo_create(ctx->gbm, ctx->w, ctx->h, 
                                    GBM_FORMAT_ARGB8888,
                                    GBM_BO_USE_LINEAR | GBM_BO_USE_SCANOUT);
        if(!ctx->linear_bos[i])
        {
            fprintf(stderr, "linear_bo null %d\n", errno);
            return -1;
        }

        ctx->linear_images[i] = eglCreateImage(ctx->d, EGL_NO_CONTEXT,
                                    EGL_NATIVE_PIXMAP_KHR, ctx->linear_bos[i], NULL);


        glGenFramebuffers(1, &ctx->linear_fbs[i]);
        glGenRenderbuffers(1, &ctx->linear_rbs[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, ctx->linear_rbs[i]);
        _mesa_EGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, ctx->linear_images[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, ctx->linear_fbs[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ctx->linear_rbs[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, ctx->tiled_fb);

    return 0;
}

int GKGLSwapBuffers(GKGLContext ctx)
{
    if(!ctx)
    {
        errno = EINVAL;
        return -1;
    }

    /* Now use the rendering pipeline to copy from the tiled buffer to
        a linear screen buffer */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx->tiled_fb);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ctx->linear_fbs[GK_GPUGetRenderBuffer()]);
    glBlitFramebuffer(0, 0, ctx->w, ctx->h, 0, 0,
        ctx->w, ctx->h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, ctx->tiled_fb);
    GK_GPUFlipScreen();

    return 0;
}

int GKGLDeleteContext(GKGLContext ctx)
{
    if(!ctx)
    {
        errno = EINVAL;
        return -1;
    }

    if(ctx->ctx)
    {
        eglDestroyContext(ctx->d, ctx->ctx);
    }

    if(ctx->gbm)
    {
        if(ctx->tiled_bo)
        {
            gbm_bo_destroy(ctx->tiled_bo);
        }
        for(int i = 0; i < 3; i++)
        {
            if(ctx->linear_bos)
            {
                gbm_bo_destroy(ctx->linear_bos[i]);
            }
        }
        gbm_device_destroy(ctx->gbm);
    }

    delete ctx;
    return 0;
}
