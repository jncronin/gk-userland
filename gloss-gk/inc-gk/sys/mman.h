#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <stddef.h>
#include <sys/types.h>

void *mmap(void *addr, size_t length, int prot, int flags,
            int fd, off_t offset);
int munmap(void *addr, size_t length);

#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       3
#define PROT_NONE       4

#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_FIXED       3
#define MAP_ANONYMOUS   4
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)



int    mlock(const void *, size_t);
int    mlockall(int);
int    mprotect(void *, size_t, int);
int    msync(void *, size_t, int);
int    munlock(const void *, size_t);
int    munlockall(void);


#endif
