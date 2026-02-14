#include "dlfcn.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#if __GAMEKID__ >= 4
#include <stdint.h>
static void * const __dl_main_exec = (void *)-1;

void *dlopen(const char *file, int mode)
{
    if(!file)
    {
        // open main exec
        return __dl_main_exec;
    }
    else
    {
        // just try and open
        auto oret = open(file, O_RDONLY);
        if(oret >= 0)
        {
            return (void *)(intptr_t)oret;
        }
        else
        {
            return nullptr;
        }
    }
}

int dlclose(void *handle)
{
    if(handle == __dl_main_exec)
    {
        return 0;
    }

    close((int)(intptr_t)handle);
    return 0;
}

void *dlsym(void *handle, const char *name)
{
    fprintf(stderr, "dlsym: %s within %p\n", name, handle);
    return nullptr;
}

static char _dlerror[] = "dlfcn.h functions are not supported";
char *dlerror()
{
    return _dlerror;
}

#else
// we don't support dl*()

void *dlopen(const char *file, int mode)
{
    fprintf(stderr, "dlopen (%s) called, failing\n", file);
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
#endif
