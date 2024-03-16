#include "args.h"
#include <iostream>

Args::Args()
{
    next_id = 1;

    adescs.push_back(arg_desc());       // entry 0 is any extra arguments
}

int Args::AddHelp()
{
    return AddArg("--help", 0, "-h", "", "show this help");
}

int Args::AddArg(std::string long_name, int nparams,
    std::string short_name,
    std::string def_value,
    std::string help_text)
{
    arg_desc ad;
    ad.def_value = def_value;
    ad.help_text = help_text;
    ad.long_name = long_name;
    ad.nparams = nparams;
    ad.short_name = short_name;

    int id = next_id++;
    adescs.push_back(ad);
    if(!long_name.empty())
        anames[long_name] = id;
    if(!short_name.empty())
        anames[short_name] = id;

    return id;
}

std::string Args::arg_desc::name() const
{
    if(!long_name.empty())
        return long_name;
    else
        return short_name;
}

std::pair<int, Args::arg_ret_t> Args::ParseArgs(int argc, const char * const argv[])
{
    arg_ret_t ret;

    int narg = 0;

    std::string fname = argv[0];
    narg++;

    bool in_named_args = true;

    while(narg < argc)
    {
        std::string curarg = argv[narg++];
        bool is_named_arg = true;

        if(curarg.empty())
        {
            continue;
        }

        auto names_iter = anames.find(curarg);
        if(names_iter == anames.end())
        {
            is_named_arg = false;
        }

        if(is_named_arg && !in_named_args)
        {
            // can't re-enter named args from args
            return { -1, ret };
        }
        else if(!is_named_arg)
        {
            in_named_args = false;
        }

        std::string arg_name = "";      // unnamed arg
        int nparams = 0;
        if(in_named_args)
        {
            // get the argument
            const auto &ad = adescs[names_iter->second];
            arg_name = ad.name();
            nparams = ad.nparams;
        }

        // add to return vectors
        if(ret.find(arg_name) == ret.end())
        {
            ret[arg_name] = std::vector<std::string>();
        }

        // add extra params, if any
        if(in_named_args)
        {
            for(int i = 0; i < nparams && narg < argc; i++)
            {
                ret[arg_name].push_back(argv[narg++]);
            }
        }
        else
        {
            // unnamed - add the exact text
            ret[arg_name].push_back(curarg);
        }
    }

    // now fill in any default params, if needed
    for(const auto &ad : adescs)
    {
        if(!ad.def_value.empty() && ret.find(ad.name()) == ret.end())
        {
            std::vector<std::string> def;
            def.push_back(ad.def_value);
            ret[ad.name()] = def;
        }
    }

    return { 0, ret };
}

static std::string pad_str(const std::string &input, size_t len)
{
    if(len <= input.length())
    {
        return input;
    }

    return input + std::string(len - input.length(), ' ');
}

void Args::ShowHelp()
{
    // display argument help text (need to prepend with "Usage argv[0] [OPTION] ..." etc)
    
    // first, find max length of short_name, long_name pair
    int max_short_len = 0;
    int max_long_len = 0;

    for(const auto &ad : adescs)
    {
        if(!ad.short_name.empty())
        {
            auto cur_short_len = ad.short_name.length();
            if(cur_short_len > max_short_len)
                max_short_len = cur_short_len;
        }
        if(!ad.long_name.empty())
        {
            auto cur_long_len = ad.long_name.length();
            if(cur_long_len > max_long_len)
                max_long_len = cur_long_len;
        }
    }

    for(const auto &ad : adescs)
    {
        std::cout << pad_str(ad.short_name, max_short_len);
        if(!ad.short_name.empty() && !ad.long_name.empty())
            std::cout << ", ";
        else
            std::cout << "  ";
        std::cout << pad_str(ad.long_name, max_long_len);

        std::cout << "  " << ad.help_text << std::endl;
    }
}