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
    unsigned int depth_size = 0;
    unsigned int stencil_size = 0;

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
    GLuint tiled_rb_depth_stencil = 0;
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
    attrs->maj_ver = 2;
    attrs->min_ver = 1;
    attrs->core_profile = 0;
    attrs->gles = 0;
    attrs->sample_buffers = 0;
    attrs->samples = 0;

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

    if(attrs->gles)
        eglBindAPI(EGL_OPENGL_ES_API);
    else
        eglBindAPI(EGL_OPENGL_API);

    GLint renderable_type = EGL_OPENGL_BIT;
    if(attrs->gles)
    {
        if(attrs->maj_ver >= 3)
            renderable_type = EGL_OPENGL_ES3_BIT;
        else if(attrs->maj_ver >= 2)
            renderable_type = EGL_OPENGL_ES2_BIT;
        else
            renderable_type = EGL_OPENGL_ES_BIT;
    }

    EGLint const attrib_list[] =
    {
        EGL_RENDERABLE_TYPE, renderable_type,
        EGL_RED_SIZE, attrs->rsize,
        EGL_GREEN_SIZE, attrs->gsize,
        EGL_BLUE_SIZE, attrs->bsize,
        EGL_ALPHA_SIZE, attrs->asize,
        EGL_DEPTH_SIZE, (attrs->depth_size <= 16) ? attrs->depth_size : 16,
        EGL_STENCIL_SIZE, (attrs->stencil_size <= 8) ? attrs->stencil_size : 8,
        EGL_SAMPLE_BUFFERS, attrs->sample_buffers,
        EGL_SAMPLES, attrs->samples,
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
        fprintf(stderr, "egl failed to match rsize: %d, gsize: %d, bsize: %d, asize: %d, depth_size: %d, stencil_size: %d, sample_buffers: %d, samples: %d\n",
            attrs->rsize, attrs->gsize, attrs->bsize, attrs->asize, attrs->depth_size, attrs->stencil_size, attrs->sample_buffers, attrs->sample_buffers);
        GKGLDeleteContext(ctx);
        return -1;
    }

    for(auto i = 0; i < nconfigs; i++)
    {
        auto &conf = configs[i];

        eglGetConfigAttrib(ctx->d, conf, EGL_RED_SIZE, &attrs->rsize);
        eglGetConfigAttrib(ctx->d, conf, EGL_GREEN_SIZE, &attrs->gsize);
        eglGetConfigAttrib(ctx->d, conf, EGL_BLUE_SIZE, &attrs->bsize);
        eglGetConfigAttrib(ctx->d, conf, EGL_ALPHA_SIZE, &attrs->asize);
        eglGetConfigAttrib(ctx->d, conf, EGL_DEPTH_SIZE, &attrs->depth_size);
        eglGetConfigAttrib(ctx->d, conf, EGL_STENCIL_SIZE, &attrs->stencil_size);
        eglGetConfigAttrib(ctx->d, conf, EGL_NATIVE_VISUAL_ID, &ctx->native_id);
        eglGetConfigAttrib(ctx->d, conf, EGL_SAMPLE_BUFFERS, &attrs->sample_buffers);
        eglGetConfigAttrib(ctx->d, conf, EGL_SAMPLES, &attrs->samples);

        if(attrs->gles)
        {
            EGLint ctx_attrib_list[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, (EGLint)attrs->maj_ver,
                EGL_NONE
            };

            ctx->ctx = eglCreateContext(ctx->d, conf, EGL_NO_CONTEXT, ctx_attrib_list);
        }
        else
        {
            EGLint ctx_attrib_list[] =
            {
                EGL_CONTEXT_MAJOR_VERSION, (EGLint)attrs->maj_ver,
                EGL_CONTEXT_MINOR_VERSION, (EGLint)attrs->min_ver,
                (attrs->maj_ver >= 3) ? EGL_CONTEXT_OPENGL_PROFILE_MASK : EGL_NONE,
                    attrs->core_profile ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
                EGL_NONE
            };

            ctx->ctx = eglCreateContext(ctx->d, conf, EGL_NO_CONTEXT, ctx_attrib_list);
        }

        if(ctx->ctx)
        {
            fprintf(stderr, "gkgl: created context:\n"
                "  RED:         %d\n"
                "  GREEN:       %d\n"
                "  BLUE:        %d\n"
                "  DEPTH:       %d\n"
                "  STENCIL:     %d\n"
                "  SBUFFERS:    %d\n"
                "  SAMPLES:     %d\n"
                "  NATIVE_ID:   %x\n",
                attrs->rsize, attrs->gsize, attrs->bsize, attrs->depth_size, attrs->stencil_size,
                attrs->sample_buffers, attrs->samples, ctx->native_id);
            break;
        }
    }

    if(ctx->ctx == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "eglCreateContext failed: %d\n", eglGetError());
        GKGLDeleteContext(ctx);
        return -1;
    }
    else
    {
        fprintf(stderr, "gkgl: created OpenGL%s context ver %u.%u\n",
            attrs->gles ? "ES" : "",
            attrs->maj_ver, attrs->min_ver);
    }

    ctx->w = attrs->width;
    ctx->h = attrs->height;
    ctx->depth_size = attrs->depth_size;
    ctx->stencil_size = attrs->stencil_size;

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
    EGLint ctx_type = 0;
    if(!eglQueryContext(ctx->d, ctx->ctx, EGL_CONTEXT_CLIENT_TYPE, &ctx_type))
    {
        fprintf(stderr, "gkgl: eglQueryContext failed: %d\n", eglGetError());
        return -1;
    }
    fprintf(stderr, "gkgl: context type: %d\n", ctx_type);
    fprintf(stderr, "gkgl: glVersion: %s\n", glGetString(GL_VERSION));

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

    // It also needs a depth + stencil buffer, if so requested
    if(ctx->depth_size || ctx->stencil_size)
    {
        glGenRenderbuffers(1, &ctx->tiled_rb_depth_stencil);
        glBindRenderbuffer(GL_RENDERBUFFER, ctx->tiled_rb_depth_stencil);

        if(ctx->depth_size == 0)
        {
            // just a stencil
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, ctx->w, ctx->h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx->tiled_rb_depth_stencil);
        }
        else if(ctx->stencil_size == 0)
        {
            // just depth
            if(ctx->depth_size <= 16)
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ctx->w, ctx->h);
            else
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ctx->w, ctx->h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx->tiled_rb_depth_stencil);
        }
        else
        {
            // both
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ctx->w, ctx->h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx->tiled_rb_depth_stencil);
        }
    }

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
#if 0
    auto blend = glIsEnabled(GL_BLEND);
    glDisable(GL_BLEND);
#endif
    glBlitFramebuffer(0, 0, ctx->w, ctx->h,
        0, ctx->h, ctx->w, 0,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, ctx->tiled_fb);
#if 0
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    auto lighting = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    if(lighting)
        glEnable(GL_LIGHTING);
    if(blend)
        glEnable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
#endif
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

    if(ctx->d)
    {
        eglMakeCurrent(ctx->d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_SURFACE);
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
            if(ctx->linear_bos[i])
            {
                gbm_bo_destroy(ctx->linear_bos[i]);
            }
        }
        gbm_device_destroy(ctx->gbm);
    }

    delete ctx;
    return 0;
}
