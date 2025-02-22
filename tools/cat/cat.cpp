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

            // only read the _current_ size of the file - so we don't keep reading syslog
            //  sending information about us reading it
            fseek(f, 0, SEEK_END);
            auto flen = ftell(f);
            fseek(f, 0, SEEK_SET);

            size_t nr = 0;

            auto cbuf = new char[4096];

            while(true)
            {
                auto tr = (unsigned long)flen - nr;
                if(tr > 4096) tr = 4096;
                if(tr == 0)
                    break;
                
                auto br = fread(cbuf, 1, (size_t)tr, f);
                if(!br)
                    break;
                fwrite(cbuf, 1, br, stdout);

                nr += br;
            }

            fclose(f);

            delete[] cbuf;
        }
    }

    return 0;
}
