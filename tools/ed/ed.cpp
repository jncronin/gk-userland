#include <unistd.h>
#include <shell.h>
#include <args.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
#include "ed.h"
#include "parser.h"

std::string readline();

SHELL_MAIN(ed)
{
    Args a;
    a.AddHelp();
    auto args = a.ParseArgs(argc, argv);

    if(args.first != 0)
        return args.first;

    // get default filename
    std::string fname;
    auto deffname = args.second.find("");
    if(deffname != args.second.end())
    {
        if(deffname->second.size() > 0)
            fname = deffname->second[0];
    }

    ed_state s;

    // load the file if possible
    if(!fname.empty())
    {
        s.push(cmd_load(fname, 0));
    }

    bool show_prompt = false;
    std::string prompt;

    // now parse commands until exit
    while(true)
    {
        if(show_prompt)
            puts(prompt.c_str());
        
        auto l = readline();

        unsigned int p = 0;
        auto a0 = parse_line_address(l, &p);
        auto a1 = parse_second_line_address(l, &p);
        auto cmd = parse_command(l, &p);
        auto params = parse_parameters(l, &p);

        auto l0 = a0.empty() ? s.cur.addr : strtoul(a0.c_str(), nullptr, 10);
    }

}

std::string readline()
{
    auto linebuf = new char[PATH_MAX];
    if(!linebuf)
    {
        printf("ed: out of memory\n");
        exit(-1);
    }
    if(fgets(linebuf, PATH_MAX - 1, stdin))
    {
        linebuf[PATH_MAX - 1] = 0;
        linebuf[strcspn(linebuf, "\r\n")] = 0;      // remove newline character(s)
        return std::string(linebuf);
    }
    return "";
}
