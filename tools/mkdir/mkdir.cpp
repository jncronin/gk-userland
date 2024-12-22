#include <unistd.h>
#include <shell.h>
#include <args.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

SHELL_MAIN(mkdir)
{
    Args a;
    a.AddHelp();
    auto args = a.ParseArgs(argc, argv);
    if(args.first != 0)
        return args.first;
    auto newdir = args.second.find("");
    if(newdir != args.second.end())
    {
        for(const auto &nd : newdir->second)
        {
            if(mkdir(nd.c_str(), 0777))
            {
                printf("mkdir: couldn't create %s (%s)\n", nd.c_str(), strerror(errno));
            }
        }
    }

    return 0;
}

