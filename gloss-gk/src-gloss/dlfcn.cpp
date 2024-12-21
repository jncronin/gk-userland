#include "dlfcn.h"

// we don't support dl*()

void *dlopen(const char *file, int mode)
{
    return nullptr;
}

int dlclose(void *handle)
{
    return -1;
}

void *dlsym(void *handle, const char *name)
{
    return nullptr;
}

static char _dlerror[] = "dlfcn.h functions are not supported";
char *dlerror()
{
    return _dlerror;
}
