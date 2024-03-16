#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <iostream>
#include "args.h"

static void show_attr(mode_t show, std::string if_true)
{
    if(show)
        std::cout << if_true;
    else
        std::cout << "-";
}

int main(int argc, char *argv[])
{
    /* Parse args */
    Args a;
    a.AddHelp();
    a.AddArg("", 0, "-l", "", "use a long listing format");

    auto args = a.ParseArgs(argc, argv);
    if(args.first != 0)
        return args.first;
    
    bool is_long = false;
    if(args.second.find("-l") != args.second.end())
        is_long = true;
    if(args.second.find("--help") != args.second.end())
    {
        a.ShowHelp();
        return 0;
    }
    
    std::vector<std::string> rel_paths;
    rel_paths.push_back(".");

    auto iter = args.second.find("");
    if(iter != args.second.end())
        rel_paths = iter->second;

    // current path
    char buf[512];
    if(!getcwd(buf, 511))
        return -1;
    buf[511] = 0;
    std::string cur_path = std::string(buf);

    for(const auto &rel_path : rel_paths)
    {
        // listing for current path
        auto act_path = cur_path + "/" + rel_path;

        auto d = opendir(act_path.c_str());
        if(!d)
            return -1;
        
        std::vector<std::string> entries;

        dirent *de;
        while(true)
        {
            de = readdir(d);
            if(!de)
                break;
            
            entries.push_back(de->d_name);
        }
        closedir(d);

        std::sort(entries.begin(), entries.end());

        for(const auto &entry : entries)
        {
            if(is_long)
            {
                struct stat st;
                if(stat((rel_path + "/" + entry).c_str(), &st) != 0)
                {
                    return -1;
                }

                if(st.st_mode & S_IFDIR)
                {
                    std::cout << "d";
                }
                else
                {
                    std::cout << "-";
                }

                show_attr(st.st_mode & S_IRUSR, "r");
                show_attr(st.st_mode & S_IWUSR, "w");
                show_attr(st.st_mode & S_IXUSR, "x");
                show_attr(st.st_mode & S_IRGRP, "r");
                show_attr(st.st_mode & S_IWGRP, "w");
                show_attr(st.st_mode & S_IXGRP, "x");
                show_attr(st.st_mode & S_IROTH, "r");
                show_attr(st.st_mode & S_IWOTH, "w");
                show_attr(st.st_mode & S_IXOTH, "x");

                std::cout << " " << std::to_string(st.st_uid) << " " << std::to_string(st.st_gid) << 
                    " " << std::to_string(st.st_size) << " " << entry << std::endl;
            }
            else
            {
                std::cout << entry << std::endl;
            }
        }
    }

    return 0;
}
