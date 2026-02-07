#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

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
        return 0;
    }
    auto d = opendir(path);
    if(d)
    {
        closedir(d);
        return 0;
    }
    return -1;
}
