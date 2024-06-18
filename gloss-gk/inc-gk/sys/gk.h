#ifndef _SYS_GK_H
#define _SYS_GK_H

// extra GK defines not added by newlib nano
#define FE_TONEAREST            0
#define FE_TOWARDZERO           1
#define FE_DOWNWARD             2
#define FE_UPWARD               3

#define __STRINGIFY(a) #a

#define __INT64 "ll"
#define __PRI64(x) __INT64 __STRINGIFY(x)

#define PRId64		__PRI64(d)
#define PRIi64		__PRI64(i)
#define PRIo64		__PRI64(o)
#define PRIu64		__PRI64(u)
#define PRIx64		__PRI64(x)
#define PRIX64		__PRI64(X)

struct stat;
int     lstat (const char * __path, struct stat * __buf );

#endif
