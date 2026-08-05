#include "timezones.h"
#include "gk.h"
#include <iostream>
#include <fstream>
#include <string_view>
#include <string>
#include "json.hpp"
#include <time.h>

using json = nlohmann::json;


using namespace std::string_view_literals;

static const constexpr auto conf_fname = "/etc/timezones.json";

int timezones_read()
{
    json j;
    std::ifstream i(conf_fname);
    if(i.is_open())
    {
        i >> j;
        i.close();

        if(j.is_object() == false)
        {
            fprintf(stderr, "timezones_read: json file is not an object\n");
            return -1;
        }

        auto tzval = j["tz"];
        if(!tzval.is_string())
        {
            fprintf(stderr, "timezones_read: tz member is not a string\n");
            return -1;
        }

        auto s_tzval = tzval.get<std::string>();

        setenv("TZ", s_tzval.c_str(), 1);
        tzset();
        GK_SetTZ(s_tzval.c_str());

        return 0;
    }
    else
    {
        fprintf(stderr, "timezones_read: %s cannot be opened for reading\n", conf_fname);
        return -1;
    }
}