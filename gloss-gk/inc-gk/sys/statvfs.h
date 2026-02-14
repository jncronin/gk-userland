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
    union
    {
        unsigned long _f_flag;
        unsigned long _f_flags;
    } __ff;
    unsigned long f_namemax;
};

#define f_flag __ff._f_flag
#define f_flags __ff._f_flags

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

#define MNT_LOCAL       1024

#endif
