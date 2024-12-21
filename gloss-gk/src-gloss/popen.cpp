#include <stdio.h>
#include <errno.h>

extern "C" FILE *popen(const char *command, const char *type)
{
    fprintf(stderr, "popen(%s, %s) - not supported\n", command, type);
    errno = ENOTSUP;
    return nullptr;
}

extern "C" int pclose(FILE *stream)
{
    errno = EINVAL;
    return -1;
}
