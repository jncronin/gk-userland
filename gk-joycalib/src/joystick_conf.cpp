#include "joystick_conf.h"
#include "gk.h"
#include <iostream>
#include <fstream>
#include <string_view>

using namespace std::string_view_literals;

static const constexpr auto conf_fname = "/etc/joystick_conf.json";
static const constexpr char * const axis_names[] =
{
    "joya", "joyb", "tilt", "throttle"
};
static constexpr size_t n_axes = sizeof(axis_names) / sizeof(axis_names[0]);

json joystick_conf_read()
{
    json j;
    std::ifstream i(conf_fname);
    if(i.is_open())
    {
        i >> j;
        i.close();
    }
    return j;
}

int joystick_conf_write(const json &j)
{
    std::ofstream o(conf_fname);
    o << std::setw(4) << j << std::endl;
    o.close();
    return 0;
}

int joystick_conf_add_calib(json &j, unsigned int axis_pair, const joystick_calib &calib)
{
    if(axis_pair >= n_axes)
    {
        return -1;
    }

    auto axis_name = axis_names[axis_pair];

    j[axis_name] = {
        { "left", calib.left },
        { "right", calib.right },
        { "top", calib.top },
        { "bottom", calib.bottom },
        { "centre_x", calib.centre_x },
        { "centre_y", calib.centre_y }
    };

    return 0;
}

int joystick_conf_apply_calib(const json &j)
{
    for(const auto &member : j.items())
    {
        // Is the member key in axis_names?
        unsigned int axis_pair = ~0U;

        for(auto i = 0U; i < n_axes; i++)
        {
            if(!strcmp(member.key().c_str(), axis_names[i]))
            {
                axis_pair = i;
                break;
            }
        }

        if(axis_pair >= n_axes)
        {
            continue;
        }

        auto left = member.value()["left"];
        auto right = member.value()["right"];
        auto top = member.value()["top"];
        auto bottom = member.value()["bottom"];
        auto centre_x = member.value()["centre_x"];
        auto centre_y = member.value()["centre_y"];

        if(!left.is_number() ||
            !right.is_number() ||
            !top.is_number() ||
            !bottom.is_number() ||
            !centre_x.is_number() ||
            !centre_y.is_number())
        {
            continue;
        }

        GK_SetJoystickCalibration(axis_pair,
            left.get<int>(),
            right.get<int>(),
            top.get<int>(),
            bottom.get<int>(),
            centre_x.get<int>(),
            centre_y.get<int>());
    }

    return 0;
}
