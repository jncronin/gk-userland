#include "syscalls.h"
#include <string.h>

char *dirname(char *path)
{
    char *last_slash = strrchr(path, '/');
    if(!last_slash)
        return path;
    
    *last_slash = 0;
    return path;
}

char *basename(char *path)
{
    char *last_slash = strrchr(path, '/');
    if(!last_slash)
        return path;
    size_t bn_len = strlen(last_slash + 1);
    memmove(path, last_slash + 1, bn_len);
    path[bn_len] = 0;
    return path;
}
