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
        ret.buf.push_back(std::string(linebuf));
    }

    printf("%ld\n", ftell(f));

    fclose(f);
    delete[] linebuf;

    return ret;
}
