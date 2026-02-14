#include "syscalls.h"
#include "deferred.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

extern "C" int mkfifo(const char *pathname, mode_t mode)
{
    __syscall_mkdir_params p { pathname, mode };
    return deferred_call(__syscall_mkfifo, &p);
}

extern "C" int mkdir(const char *pathname, mode_t mode)
{
    __syscall_mkdir_params p { pathname, mode };
    return deferred_call(__syscall_mkdir, &p);
}

extern "C" int rmdir(const char *pathname)
{
    return deferred_call(__syscall_rmdir, (void *)pathname);
}

extern "C" DIR *opendir(const char *name)
{
    auto ret = deferred_call(__syscall_opendir, (void *)name);
    if(ret < 0)
    {
        return nullptr;
    }

    auto dret = (DIR *)malloc(sizeof(DIR));
    dret->dd_fd = ret;
    
    return dret;
}

extern "C" int _close(int file);

extern "C" int closedir(DIR *dirp)
{
    if(!dirp)
    {
        errno = EBADF;
        return -1;
    }
    int ret = _close(dirp->dd_fd);
    free(dirp);
    return ret;
}

extern "C" struct dirent *readdir(DIR *dirp)
{
    if(!dirp)
    {
        errno = EBADF;
        return nullptr;
    }
    __syscall_readdir_params p { dirp->dd_fd, &dirp->dd_dirent };
    auto ret = deferred_call(__syscall_readdir, &p);
    if(ret <= 0)
    {
        // end-of-stream (errno not set) or error (errno set)
        return nullptr;
    }
    else
    {
        return &dirp->dd_dirent;
    }
}

extern "C" void rewinddir(DIR *dirp)
{
    if(!dirp)
    {
        return;
    }
    deferred_call(__syscall_rewinddir, (void *)(intptr_t)dirp->dd_fd);
}

extern "C" int dirfd(DIR *)
{
    errno = ENOTSUP;
    return -1;
}
