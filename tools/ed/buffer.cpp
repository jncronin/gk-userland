#include "ed.h"

ed_buffer_state::ed_buffer_state()
{
    addr = 0;
}

ed_buffer_state::ed_buffer_state(const ed_buffer_state &other)
{
    // copy constructor - need deep copy of the line buffer
    addr = other.addr;
    markline = other.markline;
    modified = other.modified;
    fname = other.fname;
    buf.clear();
    buf = other.buf;
    cutbuf = other.cutbuf;
    qe_warn = false;    // always reset because only q and e will test and will do so before reset
}

ed_buffer_state::ed_buffer_state(ed_buffer_state &&other)
{
    // move constructor - used to swap undo buffers
    addr = other.addr;
    markline = std::move(other.markline);
    modified = other.modified;
    fname = std::move(other.fname);
    buf = std::move(other.buf);
    cutbuf = std::move(other.cutbuf);
    qe_warn = false;    // always reset because only q and e will test and will do so before reset
}

ed_buffer_state ed_buffer_state::operator=(ed_buffer_state &&other)
{
    addr = other.addr;
    markline = std::move(other.markline);
    modified = other.modified;
    fname = std::move(other.fname);
    buf = std::move(other.buf);
    cutbuf = std::move(other.cutbuf);
    qe_warn = false;    // always reset because only q and e will test and will do so before reset
    return *this;
}

void ed_state::push(ed_buffer_state &&other)
{
    prev = std::move(cur);
    cur = std::move(other);
}

void ed_state::undo()
{
    auto tmp = std::move(cur);
    cur = std::move(prev);
    prev = std::move(tmp);
}

ed_state::operator ed_buffer_state &()
{
    return cur;
}
