#include "ed.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

ed_buffer_state cmd_load(const std::string &fname, unsigned int addr)
{
    auto f = fopen(fname.c_str(), "r");
    if(!f)
    {
        printf("%s: %s\n", fname.c_str(), strerror(errno));
        return ed_buffer_state();
    }

    auto linebuf = new char[PATH_MAX];
    if(!linebuf)
    {
        printf("ed: failed to allocate buffer\n");
        exit(-1);
    }

    ed_buffer_state ret;

    while(fgets(linebuf, PATH_MAX - 1, f))
    {
        linebuf[PATH_MAX - 1] = 0;
        linebuf[strcspn(linebuf, "\r\n")] = 0;      // remove newline character(s)
        ret.buf.push_back(std::string(linebuf));
    }

    printf("%ld\n", ftell(f));

    fclose(f);
    delete[] linebuf;

    ret.addr = 1;
    ret.fname = fname;

    return ret;
}

bool cmd_write(ed_state &s, unsigned int a0, unsigned int a1, const std::string &fname,
    bool is_append, unsigned int *last_line_affected)
{
    if(a0 == 0 || a1 < a0 || a0 > s.cur.buf.size() || a1 > s.cur.buf.size())
    {
        printf("?\n");
        return false;
    }

    auto fmode = is_append ? "a" : "w";
    auto f = fopen(fname.c_str(), fmode);
    if(!f)
    {
        return false;
    }

    for(auto iter = s.cur.buf.begin() + (a0-1); iter < s.cur.buf.begin() + a1; iter++)
    {
        const auto &cline = *iter;
        fwrite(cline.c_str(), 1, cline.size(), f);
        fputc('\n', f);
    }

    fclose(f);

    s.cur.modified = false;

    return true;
}
