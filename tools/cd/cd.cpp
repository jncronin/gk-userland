#include <unistd.h>
#include <shell.h>
#include <args.h>
#include <limits.h>

SHELL_MAIN(cd)
{
    Args a;
    a.AddHelp();
    auto args = a.ParseArgs(argc, argv);
    if(args.first != 0)
        return args.first;
    auto newdir = args.second.find("");
    if(newdir != args.second.end())
    {
        if(newdir->second.size() > 0)
        {
            const auto &nd = newdir->second[0];
            return chdir(nd.c_str());
        }
    }
    auto curdir = new char[PATH_MAX];
    if(curdir)
    {
        if(getcwd(curdir, PATH_MAX - 1))
        {
            curdir[PATH_MAX - 1] = 0;
            puts(curdir);
        }
        delete[] curdir;
    }
    return 0;
}
