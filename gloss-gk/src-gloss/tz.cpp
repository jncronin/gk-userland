#include <syscalls.h>
#include "deferred.h"
#include <errno.h>
#include "gk.h"

int GK_SetTZ(const char *tzval)
{
    return deferred_call(__syscall_settz, (void *)tzval);
}
