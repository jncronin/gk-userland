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
    bool show_help = false;
    std::string prompt = "*";

    // now parse commands until exit
    while(true)
    {
        if(show_prompt)
            printf("%s", prompt.c_str());
        
        auto l = readline();

        unsigned int p = 0;
        parser_buffer_desc origbuf(s.cur);
        auto a0 = parse_line_address(l, &p, origbuf);
        parser_buffer_desc second_buf = a0.valid ? a0.bufdesc : origbuf;
        auto a1 = parse_second_line_address(l, &p, origbuf, second_buf);
        auto cmd = parse_command(l, &p, second_buf);
        if(p != l.length()) cmd.valid = false;

        // check addresses are valid
        if(a0.valid && a0.bufdesc.cline > a0.bufdesc.nlines)
            cmd.valid = false;
        if(a1.valid && a1.line.valid && a1.line.bufdesc.cline > a1.line.bufdesc.nlines)
            cmd.valid = false;

        if(cmd.valid)
        {
            // run command
            //
            switch(cmd.cmd)
            {
                case p_print:
                case n_number:
                case l_list:
                    {
                        unsigned int a0_addr, a1_addr;
                        if(interpret_address_pair(origbuf, a0, a1, &a0_addr, &a1_addr))
                        {
                            cmd_print(s, a0_addr, a1_addr,
                                cmd.cmd == n_number || ((cmd.print_suffix & PRINT_SUFFIX_N) != 0),
                                cmd.cmd == l_list || ((cmd.print_suffix & PRINT_SUFFIX_L) != 0));
                        }
                    }
                    break;
            }
        }
        else
        {
            if(show_help)
            {
                // TODO
            }
            else
            {
                puts("?");
            }
        }
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
