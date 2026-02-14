#include <unistd.h>

extern "C" ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    auto curr = lseek(fd, 0, SEEK_CUR);
    if(curr < 0)
    {
        return -1;
    }

    auto sr = lseek(fd, offset, SEEK_SET);
    if(sr < 0)
    {
        return -1;
    }

    auto rr = read(fd, buf, count);
    lseek(fd, curr, SEEK_SET);

    return rr;
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    auto curr = lseek(fd, 0, SEEK_CUR);
    if(curr < 0)
    {
        return -1;
    }

    auto sr = lseek(fd, offset, SEEK_SET);
    if(sr < 0)
    {
        return -1;
    }

    auto rr = write(fd, buf, count);
    lseek(fd, curr, SEEK_SET);

    return rr;
}
