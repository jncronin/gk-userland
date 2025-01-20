#include <stdio.h>
#include <mntent.h>
#include <errno.h>

extern "C" FILE *setmntent(const char *filename, const char *type)
{
    errno = ENOTSUP;
    return nullptr;
}

extern "C" struct mntent *getmntent(FILE *stream)
{
    errno = ENOTSUP;
    return nullptr;
}

extern "C" int addmntent(FILE *stream, const struct mntent *mnt)
{
    errno = ENOTSUP;
    return 1;
}

extern "C" int endmntent(FILE *stream)
{
    return 1;
}

extern "C" char *hasmntopt(const struct mntent *mntent, const char *opt)
{
    return nullptr;
}
