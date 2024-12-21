#include <unistd.h>

extern "C" int fsync(int fd)
{
    return 0;
}
