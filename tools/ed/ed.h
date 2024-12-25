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

ed_buffer input_mode();

ed_buffer_state cmd_load(const std::string &fname, unsigned int addr);
bool cmd_print(ed_state &s, unsigned int a0, unsigned int a1, bool number = false, bool list = false);
bool cmd_delete(ed_state &s, unsigned int a0, unsigned int a1, unsigned int *last_line_affected = nullptr);
bool cmd_change(ed_state &s, unsigned int a0, unsigned int a1, const ed_buffer &input, unsigned int *last_line_affected = nullptr);
bool cmd_append(ed_state &s, unsigned int a0, const ed_buffer &input, bool is_insert = false, unsigned int *last_line_affected = nullptr);

#endif
