#include <unistd.h>
#include <shell.h>
#include <args.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

static void delete_recursive(const std::string &file, const std::string &cur_dir, bool is_recursive);

SHELL_MAIN(rm)
{
    Args a;
    a.AddHelp();
    a.AddArg("", 0, "-r", "", "recursive");
    auto args = a.ParseArgs(argc, argv);
    auto is_recursive = args.second.find("-r") != args.second.end();

    if(args.first != 0)
        return args.first;
    auto newdir = args.second.find("");
    if(newdir != args.second.end())
    {
        for(const auto &nd : newdir->second)
        {
            delete_recursive(nd, "", is_recursive);
        }
    }
    return 0;
}

void delete_recursive(const std::string &fname, const std::string &curdir, bool is_recursive)
{
    auto curfname = curdir.empty() ? fname : (curdir + "/" + fname);

    // is the current file a directory?
    struct stat st;
    if(lstat(curfname.c_str(), &st))
    {
        printf("rm: couldn't read \'%s\' (%s)\n", curfname.c_str(), strerror(errno));
        return;
    }

    if(S_ISDIR(st.st_mode))
    {
        if(!is_recursive)
        {
            printf("rm: cannot remove \'%s\': Is a directory\n", curfname.c_str());
        }
        else
        {
            // iterate directory contents 
            DIR *dp;
            dp = opendir(curfname.c_str());
            if(dp == NULL)
            {
                printf("rm: cannot recurse \'%s\': %s\n", curfname.c_str(), strerror(errno));
            }
            else
            {
                struct dirent *entry;
                while((entry = readdir(dp)))
                {
                    auto cname = std::string(entry->d_name);
                    if(cname == "." || cname == "..")
                        continue;
                    delete_recursive(cname, curfname, is_recursive);
                }
                closedir(dp);
            }

            // delete directory itself
            if(rmdir(curfname.c_str()))
            {
                printf("rm: cannot remove '%s': %s\n", curfname.c_str(), strerror(errno));
            }
        }
    }
    else
    {
        // its a regular file - call unlink
        if(unlink(curfname.c_str()))
        {
            printf("rm: cannot remove '%s': %s\n", curfname.c_str(), strerror(errno));
        }
    }
}
