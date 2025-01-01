#include "syscalls.h"
#include "deferred.h"

extern "C" int GK_SetSupervisorVisible(int visible, int screen)
{
    __syscall_setsupervisorvisible_params p { .visible = visible, .screen = screen };
    return deferred_call(__syscall_setsupervisorvisible, &p);
}
