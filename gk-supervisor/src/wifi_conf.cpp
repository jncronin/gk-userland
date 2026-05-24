#include "wifi_conf.h"
#include "gk.h"
#include <iostream>
#include <fstream>
#include <string_view>

using namespace std::string_view_literals;

static const constexpr auto conf_fname = "/etc/wifi_conf.json";

json wifi_conf_read()
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

int wifi_conf_write(const json &j)
{
    std::ofstream o(conf_fname);
    if(o.is_open() == false)
    {
        fprintf(stderr, "wifi_conf_write: could not open %s for writing\n", conf_fname);
        return -1;
    }
    o << std::setw(4) << j << std::endl;
    o.close();
    return 0;
}

int wifi_conf_apply(const json &j)
{
    if(j.is_array() == false)
    {
        fprintf(stderr, "wifi_conf_apply: json file is not an array\n");
        return -1;
    }

    GK_WifiClearNetworks();

    for(const auto &network : j.items())
    {
        if(network.value().is_object() == false)
        {
            fprintf(stderr, "wifi_conf_apply: json member is not an object\n");
            continue;
        }

        auto ssid = network.value()["ssid"];
        auto ch = network.value()["ch"];
        
        if(!ssid.is_string())
        {
            fprintf(stderr, "wifi_conf_apply: invalid ssid\n");
            continue;
        }

        auto s_ssid = ssid.get<std::string>();

        auto int_ch = ch.is_number_integer() ? ch.get<int>() : -1;

        auto cred_type = network.value()["cred_type"];
        if(cred_type.is_string() && strcmp("open", cred_type.get<std::string>().c_str()))
        {
            auto s_cred_type = cred_type.get<std::string>();
            if(!strcmp("psk", s_cred_type.c_str()))
            {
                auto psk = network.value()["psk"];
                if(!psk.is_string())
                {
                    fprintf(stderr, "wifi_conf_apply: no psk in network with cred_type psk\n");
                    continue;
                }

                auto s_psk = psk.get<std::string>();

                GK_WifiAddPSKNetwork(s_ssid.c_str(), int_ch, s_psk.c_str());
            }
        }
        else
        {
            GK_WifiAddOpenNetwork(s_ssid.c_str(), int_ch);
        }
    }

    return 0;
}
