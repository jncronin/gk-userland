#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/time.h>
#include <sys/types.h>

#define RUSAGE_SELF     0               /* calling process */
#define RUSAGE_CHILDREN -1              /* terminated child processes */

#define PRIO_PROCESS    0
#define PRIO_PGRP       1
#define PRIO_USER       2

typedef unsigned int rlim_t;

#define RLIM_INFINITY   0
#define RLIM_SAVED_MAX  1
#define RLIM_SAVED_CUR  2

struct rlimit
{
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

struct rusage {
        struct timeval ru_utime;        /* user time used */
        struct timeval ru_stime;        /* system time used */
};

int  getpriority(int, id_t);
int  getrlimit(int, struct rlimit *);
int  getrusage(int, struct rusage *);
int  setpriority(int, id_t, int);
int  setrlimit(int, const struct rlimit *);

#define RLIMIT_CORE     0
#define RLIMIT_CPU      1
#define RLIMIT_DATA     2
#define RLIMIT_FSIZE    3
#define RLIMIT_NOFILE   4
#define RLIMIT_STACK    5
#define RLIMIT_AS       6

#endif
