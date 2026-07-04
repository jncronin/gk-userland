/* Patch in the required functions for SDL2.  This ensures that they are all included in
    the statically linked executables. */

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <cstddef>

struct RenderDataFunctionsPart
{
#define SDL_PROC(ret, func, params) ret (APIENTRY *func) params;
#include "SDL_glfuncs.h"
#undef SDL_PROC
};

static_assert(offsetof(RenderDataFunctionsPart, glBegin) == 0);

extern "C" int GKGLLoadSDLFunctions(RenderDataFunctionsPart *data)
{
#define SDL_PROC(ret, func, params) data->func = func;
#include "SDL_glfuncs.h"
#undef SDL_PROC
    return 0;
}
