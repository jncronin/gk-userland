#ifndef ED_H
#define ED_H

#include <string>
#include <vector>

using ed_buffer = std::vector<std::string>;

struct ed_buffer_state
{
    ed_buffer buf, cutbuf;
    unsigned int addr;
    int markline;

    ed_buffer_state(const ed_buffer_state &other);
    ed_buffer_state(ed_buffer_state &&other);
    ed_buffer_state();

    ed_buffer_state operator=(ed_buffer_state &&other);
};

struct ed_state
{
    ed_buffer_state cur, prev;
    void push(ed_buffer_state &&other);
    void undo();

    operator ed_buffer_state &();
};

ed_buffer_state cmd_load(const std::string &fname, unsigned int addr);

#endif
