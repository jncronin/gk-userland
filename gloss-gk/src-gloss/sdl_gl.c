#include <stdio.h>
#include <stdlib.h>

static void GK_GL_NotLinked()
{
    fprintf(stderr, "error: GL functions requested but not linked with -lGL\n");
    exit(-1);
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
