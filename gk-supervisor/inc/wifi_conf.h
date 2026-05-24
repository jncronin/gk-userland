#ifndef WIFI_CONF_H
#define WIFI_CONF_H

#include "json.hpp"

using json = nlohmann::json;

json wifi_conf_read();
int wifi_conf_apply(const json &j);

#endif
