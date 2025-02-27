#include <cstdio>
#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include "shell.h"
#include "args.h"

#define LINE_MAX    1024

using main_type = int (*)(int, const char *[], shell_state *);

#define ADD_BUILTIN(x) \
    int x##_main(int, const char *[], shell_state *); \
    builtins[#x] = x##_main;

std::vector<std::string> tokenize(const char *buf, unsigned int len);
std::map<std::string, main_type> builtins;

int sh_main(int argc, const char *argv[], shell_state *sst);
static int exit_main(int argc, const char *argv[], shell_state *sst);

#ifdef IN_SHELL
int main(int argc, const char *argv[])
{
    return sh_main(argc, argv, nullptr);
}
#endif

int sh_main(int argc, const char *argv[], shell_state *sst)
{
    // sh always generates a new shell state
    shell_state _sst;
    sst = &_sst;

    // parse arguments
    Args a;
    a.AddHelp();
    a.AddArg("", 0, "-r", "", "don't exit on EOF on stdin");
    auto args = a.ParseArgs(argc, argv);
    auto dont_exit_on_eof = args.second.find("-r") != args.second.end();

    // builtins
    ADD_BUILTIN(ls);
    ADD_BUILTIN(sh);
    ADD_BUILTIN(exit);
    ADD_BUILTIN(cat);
    ADD_BUILTIN(cd);
    ADD_BUILTIN(mkdir);
    ADD_BUILTIN(rm);
    ADD_BUILTIN(ed);

    char linebuf[LINE_MAX];

    while(sst->to_exit == false)
    {
        // prompt
        char cwd[LINE_MAX];
        cwd[0] = 0;
        getcwd(cwd, LINE_MAX - 1);
        cwd[LINE_MAX - 1] = 0;

        std::string prompt = "user@gk:" + std::string(cwd) + "$ ";
        fputs(prompt.c_str(), stdout);
        fflush(stdout);

        if(fgets(linebuf, LINE_MAX-1, stdin))
        {
            linebuf[LINE_MAX - 1] = 0;

            auto toks = tokenize(linebuf, LINE_MAX);

            if(toks.size() > 0)
            {
                const auto &cmd = toks[0];
                
                // ?interpret as built-in
                auto iter = builtins.find(cmd);
                if(iter == builtins.end() && cmd.rfind("/bin/", 0) == 0)
                {
                    iter = builtins.find(cmd.substr(5));
                }
                if(iter != builtins.end())
                {
                    auto builtin = iter->second;

                    // build args array
                    auto argarray = new const char *[toks.size()];
                    for(unsigned int i = 0; i < toks.size(); i++)
                    {
                        argarray[i] = toks[i].c_str();
                    }

                    builtin(toks.size(), argarray, sst);
                    delete[] argarray;
                }
                else
                {
                    // try and call
                    printf("builtin not found: %s\n", cmd.c_str());
                }
            }
        }
        else if(!dont_exit_on_eof)
        {
            sst->exit_code = -1;
            sst->to_exit = true;
        }
    }

    return sst->exit_code;
}

int exit_main(int argc, const char *argv[], shell_state *sst)
{
    if(argc > 1)
    {
        sst->exit_code = strtol(argv[1], nullptr, 10);
    }
    sst->to_exit = true;
    return sst->exit_code;
}
