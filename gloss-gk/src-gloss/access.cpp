#include <unistd.h>
#include <fcntl.h>

extern "C" int access(const char *path, int mode)
{
    int fmode = 0;
    if(mode == W_OK)
    {
        fmode = O_RDWR;
    }
    else
    {
        fmode = O_RDONLY;
    }
    auto fd = open(path, fmode);
    if(fd >= 0)
    {
        close(fd);
    }
    return (fd >= 0) ? 0 : -1;
}
