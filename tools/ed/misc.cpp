#include "ed.h"

bool cmd_quit(ed_state &s, bool unconditional)
{
    if(!unconditional && s.cur.modified && !s.cur.qe_warn)
    {
        s.cur.qe_warn = true;
        printf("?\n");
        return false;
    }
    return true;
}
