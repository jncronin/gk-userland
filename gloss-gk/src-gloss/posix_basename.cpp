#include <libgen.h>
#include <errno.h>
#include <string.h>

/* This is the POSIX version of basename.

    POSIX basename modifies its argument. */

extern "C" char *basename(char *path);

char *basename(char *path)
{
    if(!path)
    {
        errno = EINVAL;
        return nullptr;
    }

    if(!strcmp("/", path))
        return path;
    
    auto *last_slash = strrchr(path, '/');

    if(!last_slash)
        return path;
    
    size_t bn_len = strlen(last_slash + 1);
    memmove(path, last_slash + 1, bn_len);
    path[bn_len] = 0;
    return path;
}
