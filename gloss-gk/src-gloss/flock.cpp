#include "syscalls.h"
#include "deferred.h"
#include <stdio.h>

extern "C" int flock(int fd, int operation)
{
    // not implemented
    return 0;
}

extern "C" void flockfile(FILE *f)
{
    // not implemented
}

extern "C" void funlockfile(FILE *f)
{
    // not implemented
}
