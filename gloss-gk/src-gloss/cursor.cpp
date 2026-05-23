#include "syscalls.h"
#include "deferred.h"
#include "gk.h"

int GK_SetCursor(int fd, unsigned int w, unsigned int h, unsigned int hx, unsigned int hy,
    unsigned int alpha, unsigned int pf, unsigned int stride)
{
    
    __syscall_setcursor_params p;
    p.fd = fd;
    p.w = w;
    p.h = h;
    p.hx = hx;
    p.hy = hy;
    p.alpha = alpha;
    p.pf = pf;
    p.stride = stride;
    
    return deferred_call(__syscall_setcursor, &p);
}

int GK_WarpCursor(unsigned int x, unsigned int y)
{
    __syscall_warpcursor_params p;
    p.x = x;
    p.y = y;

    return deferred_call(__syscall_warpcursor, &p);
}
