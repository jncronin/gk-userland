#include <unistd.h>

extern "C" int access(const char *path, int mode)
{
    return -1;
}
