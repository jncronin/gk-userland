#include <cstdio>
#include <vector>
#include <string>
#include <map>
#include <unistd.h>

#define LINE_MAX    1024
char linebuf[LINE_MAX];

using main_type = int (*)(int, const char *[]);

#define ADD_BUILTIN(x) \
    int x##_main(int, const char *[]); \
    builtins[#x] = x##_main;

std::vector<std::string> tokenize(const char *buf, unsigned int len);
std::map<std::string, main_type> builtins;

int main(int argc, char *argv[])
{
    // builtins
    ADD_BUILTIN(ls);

    while(true)
    {
        // prompt
        char cwd[LINE_MAX];
        cwd[0] = 0;
        getcwd(cwd, LINE_MAX - 1);
        cwd[LINE_MAX - 1] = 0;

        std::string prompt = "user@gk:" + std::string(cwd) + "$ ";
        fputs(prompt.c_str(), stdout);

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

                    builtin(toks.size(), argarray);
                    delete[] argarray;
                }
                else
                {
                    // try and call
                    printf("builtin not found: %s\n", cmd.c_str());
                }
            }
        }
    }
}