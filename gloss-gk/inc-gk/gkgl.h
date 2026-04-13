#ifndef GKGL_H
#define GKGL_H

struct gkgl_ctx;
typedef struct gkgl_ctx *GKGLContext;

struct GKGLAttribs
{
    unsigned int width, height;
    int rsize, gsize, bsize, asize;
    int depth_size, stencil_size;
    unsigned int maj_ver, min_ver;
    unsigned int core_profile;
};

#ifdef __cplusplus
extern "C" {
#endif

int GKGLAttribInit(struct GKGLAttribs *attrs);
int GKGLCreateContext(GKGLContext *ctx, struct GKGLAttribs *attrs);
int GKGLDeleteContext(GKGLContext ctx);
int GKGLMakeCurrent(GKGLContext ctx);
int GKGLSwapBuffers(GKGLContext ctx);

#ifdef __cplusplus
}
#endif

#endif
