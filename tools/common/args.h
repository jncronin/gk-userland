#ifndef ARGS_H
#define ARGS_H
#include <string>
#include <map>
#include <vector>

class Args
{
    protected:
        struct arg_desc
        {
            std::string long_name;
            std::string short_name;
            int nparams;
            std::string def_value;
            std::string help_text;

            std::string name() const;
        };

        std::vector<arg_desc> adescs;
        std::map<std::string, int> anames;

        int next_id;

    public:
        int AddArg(std::string long_name,
            int nparams = 0,
            std::string short_name = std::string(),
            std::string def_value = std::string(),
            std::string help_text = std::string());
        int AddHelp();
        void ShowHelp();

        Args();

        typedef std::map<std::string, std::vector<std::string>> arg_ret_t;

        std::pair<int, arg_ret_t> ParseArgs(int argc, const char * const argv[]);
};



#endif
