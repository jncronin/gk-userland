#include "ed.h"

bool cmd_delete(ed_state &s, unsigned int a0, unsigned int a1, unsigned int *last_line_affected)
{
    if(a0 == 0 || a1 < a0)
        return false;
    if(a0 > s.cur.buf.size())
        return false;
    if(a1 > s.cur.buf.size())
        a1 = s.cur.buf.size();
    
    auto newbuf = s.cur;
    newbuf.cutbuf.clear();
    newbuf.cutbuf.insert(newbuf.cutbuf.begin(), newbuf.buf.begin() + (a0-1), newbuf.buf.begin() + a1);
    newbuf.buf.erase(newbuf.buf.begin() + (a0-1), newbuf.buf.begin() + a1);
    newbuf.addr = a0;   // new address after last line deleted i.e. start address
    if(newbuf.buf.empty())
        newbuf.addr = 0;

    if(last_line_affected)
        *last_line_affected = newbuf.addr;

    s.push(std::move(newbuf));

    return true;
}

bool cmd_change(ed_state &s, unsigned int a0, unsigned int a1, const ed_buffer &input, unsigned int *last_line_affected)
{
    if(a0 == 0 || a1 < a0)
        return false;
    if(a0 > s.cur.buf.size())
        return false;
    if(a1 > s.cur.buf.size())
        a1 = s.cur.buf.size();
    
    auto newbuf = s.cur;
    newbuf.cutbuf.clear();
    newbuf.cutbuf.insert(newbuf.cutbuf.begin(), newbuf.buf.begin() + (a0-1), newbuf.buf.begin() + a1);
    auto iter = newbuf.buf.erase(newbuf.buf.begin() + (a0-1), newbuf.buf.begin() + a1);
    newbuf.buf.insert(iter, input.begin(), input.end());
    newbuf.addr = a0 + input.size();
    if(newbuf.buf.empty())
        newbuf.addr = 0;

    if(last_line_affected)
        *last_line_affected = newbuf.addr;

    s.push(std::move(newbuf));

    return true;
}

bool cmd_append(ed_state &s, unsigned int a0, const ed_buffer &input, bool is_insert, unsigned int *last_line_affected)
{
    if(is_insert && a0 > (s.cur.buf.size() + 1))
        return false;
    else if(!is_insert && a0 > s.cur.buf.size())
        return false;

    if(a0 > 0 && is_insert) a0--;

    auto newbuf = s.cur;
    newbuf.buf.insert(newbuf.buf.begin() + a0, input.begin(), input.end());
    newbuf.addr = a0 + input.size();

    if(last_line_affected)
        *last_line_affected = newbuf.addr;
    
    s.push(std::move(newbuf));

    return true;
}
