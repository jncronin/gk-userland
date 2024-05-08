#include "syscalls.h"
#include "deferred.h"

extern "C" int ftruncate(int fd, off_t length)
{
    __syscall_ftruncate_params p { fd, length };
    return deferred_call(__syscall_ftruncate, &p);
}
