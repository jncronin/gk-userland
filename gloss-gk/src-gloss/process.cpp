#include "syscalls.h"
#include "deferred.h"
#include "gk.h"

pid_t GK_GetFocusProcess()
{
    void *p = nullptr;
    return deferred_call(__syscall_getfocusprocess, p);
}

int GK_GPUGetScreenModeForProcess(pid_t pid, size_t *w, size_t *h, unsigned int *pf, int *refresh)
{
    struct __syscall_getscreenmodeforprocess_params p;
    p.pid = pid;
    p.w = w;
    p.h = h;
    p.pf = pf;
    p.refresh = refresh;
    return deferred_call(__syscall_getscreenmodeforprocess, &p);
}

int GK_GetProcessName(pid_t pid, char *name, size_t len)
{
    struct __syscall_getprocessname_params p;
    p.pid = pid;
    p.name = name;
    p.len = len;
    return deferred_call(__syscall_getprocessname, &p);
}

int GK_SetSupervisorVisibleEx(int visible, const struct gk_supervisor_visible_region *regs, size_t nregs)
{
    struct __syscall_setsupervisorvisibleex_params p;
    p.visible = visible;
    p.regs = regs;
    p.nregs = nregs;
    return deferred_call(__syscall_setsupervisorvisibleex, &p);
}
