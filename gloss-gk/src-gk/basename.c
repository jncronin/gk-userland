#include "syscalls.h"
#include <string.h>
#include <stdlib.h>

char *dirname(char *path)
{
    char *last_slash = strrchr(path, '/');
    if(!last_slash)
        return path;
    
    *last_slash = 0;
    return path;
}

char *basename(const char *_path)
{
    char *path = (char *)malloc(strlen(_path) + 1);
    strcpy(path, _path);
    char *last_slash = strrchr(path, '/');
    if(!last_slash)
        return path;
    size_t bn_len = strlen(last_slash + 1);
    memmove(path, last_slash + 1, bn_len);
    path[bn_len] = 0;
    return path;
}
