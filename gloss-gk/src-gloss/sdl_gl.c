#include <stdio.h>
#include <stdlib.h>

static void GK_GL_NotLinked()
{
    fprintf(stderr, "error: GL functions requested but not linked with -lGL\n");
    exit(-1);
}

static void GK_GKGL_NotLinked()
{
    fprintf(stderr, "error: GL functions requested but not linked with -lGKGL -lGL\n");
}

__attribute__((weak)) void OSMesaGetProcAddress()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaCreateContextAttribs()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaCreateContext()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaMakeCurrent()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaPixelStore()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaDestroyContext()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaNemaEndFrame()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void OSMesaEnableNema()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) void glFlush()
{
    GK_GL_NotLinked();
}

__attribute__((weak)) int GKGLAttribInit()
{
    GK_GKGL_NotLinked();
    return -1;
}

__attribute__((weak)) int GKGLCreateContext()
{
    GK_GKGL_NotLinked();
    return -1;
}

__attribute__((weak)) int GKGLMakeCurrent()
{
    GK_GKGL_NotLinked();
    return -1;
}

__attribute__((weak)) int GKGLSwapBuffers()
{
    GK_GKGL_NotLinked();
    return -1;
}

__attribute__((weak)) int GKGLDeleteContext()
{
    GK_GKGL_NotLinked();
    return -1;
}

__attribute__((weak)) int GKGLLoadSDLFunctions(void *)
{
    return -1;
}
