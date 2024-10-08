#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H

#include <sys/types.h>

struct statvfs
{
    unsigned long f_bsize;
    unsigned long f_frsize;
    fsblkcnt_t  f_blocks;
    fsblkcnt_t  f_bfree;
    fsblkcnt_t  f_bavail;
    fsfilcnt_t  f_files;
    fsfilcnt_t  f_ffree;
    fsfilcnt_t  f_favail;
    unsigned long f_fsid;
    unsigned long f_flag;
    unsigned long f_namemax;
};

#ifdef __cplusplus
extern "C" {
#endif

int statvfs(const char *path, struct statvfs *buf);
int fstatvfs(int, struct statvfs *);

#ifdef __cplusplus
}
#endif

#define ST_RDONLY       1
#define ST_NOSUID       2

#endif
