#include <stdio.h>
#include <shell.h>
#include <args.h>

SHELL_MAIN(cat)
{
    /* Parse arguments */
    Args a;
    a.AddHelp();

    auto args = a.ParseArgs(argc, argv);
    if(args.first != 0)
        return args.first;
    auto files = args.second.find("");
    if(files != args.second.end())
    {
        for(const auto &fname : files->second)
        {
            auto f = fopen(fname.c_str(), "r");
            if(!f)
            {
                printf("cat: %s: No such file or directory\n", fname.c_str());
                continue;
            }
            auto cbuf = new char[4096];

            while(true)
            {
                auto br = fread(cbuf, 1, 4096, f);
                if(!br)
                    break;
                fwrite(cbuf, 1, br, stdout);
            }

            delete[] cbuf;
        }
    }

    return 0;
}
