#ifndef SHELL_H
#define SHELL_H

#if defined(IN_SHELL) || defined(IN_TELNETD) || defined(IN_CONSOLE)
#define SHELL_MAIN(x) int x##_main(int argc, const char *argv[], shell_state *_sst)
#else
#define SHELL_MAIN(x) int main(int argc, char *argv[])
#endif

struct shell_state
{
    bool to_exit = false;
    int exit_code = 0;
};

#endif
