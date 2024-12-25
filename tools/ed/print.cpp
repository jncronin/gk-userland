#include "ed.h"

bool cmd_print(ed_state &s, unsigned int a0, unsigned int a1, bool number, bool list)
{
    if(a0 == 0 || a1 < a0 || a0 > s.cur.buf.size() || a1 > s.cur.buf.size())
    {
        printf("?\n");
        return false;
    }

    for(unsigned int a = a0; a <= a1; a++)
    {
        if(number)
            printf("%u\t", a);
        // TODO: escape codes for list format
        printf("%s", s.cur.buf[a-1].c_str());
        if(list)
            printf("$");
        printf("\n");
    }

    auto newbuf = s.cur;
    newbuf.addr = a1;
    s.push(std::move(newbuf));

    return true;
}
