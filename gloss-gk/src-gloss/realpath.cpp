#include "syscalls.h"
#include "deferred.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

extern "C" char *realpath(const char *path, char *resolved_path)
{
    if(!path)
    {
        errno = EINVAL;
        return nullptr;
    }

    if(!resolved_path)
    {
        resolved_path = reinterpret_cast<char *>(malloc(PATH_MAX));
        if(!resolved_path)
        {
            errno = ENOMEM;
            return nullptr;
        }
    }

    __syscall_realpath_params p { .path = path, .resolved_path = resolved_path, .len = PATH_MAX };
    auto ret = deferred_call(__syscall_realpath, &p);
    if(ret < 0)
    {
        return nullptr;
    }
    else
    {
        return resolved_path;
    }
}
