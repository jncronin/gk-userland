#ifndef JOYSTICK_CONF_H
#define JOYSTICK_CONF_H

#include "json.hpp"

using json = nlohmann::json;

struct joystick_calib
{
    int left, right, top, bottom, centre_x, centre_y;
};

json joystick_conf_read();
int joystick_conf_write(const json &j);
int joystick_conf_add_calib(json &j, unsigned int axis_pair, const joystick_calib &calib);
int joystick_conf_apply_calib(const json &j);

#endif