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

#ifndef SSIZE_MAX
#define SSIZE_MAX       INT_MAX
#endif

#ifndef SIZE_MAX
#define SIZE_MAX        UINT_MAX
#endif

#ifndef UINT64_MAX
#define UINT64_MAX      0xffffffffffffffffULL
#endif

#ifndef INT64_MAX
#define INT64_MAX       0x7fffffffffffffffLL
#endif

#ifndef INT64_MIN
#define INT64_MIN       ((int64_t)0x8000000000000000ULL)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX      UINT64_MAX
#endif

#ifndef LLONG_MAX
#define LLONG_MAX       INT64_MAX
#endif

#ifndef LLONG_MIN
#define LLONG_MIN       INT64_MIN
#endif

#ifndef PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_NORMAL 1
#endif
#ifndef PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_ERRORCHECK 2
#endif
#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE 3
#endif
#ifndef PTHREAD_MUTEX_DEFAULT
#define PTHREAD_MUTEX_DEFAULT 0
#endif

#endif
