#include <syscalls.h>
#include "deferred.h"
#include <errno.h>
#include "gk.h"

int GK_WifiClearNetworks()
{
    int na = 0;
    return deferred_call(__syscall_wifi_clearknownnetworks, &na);
}

int GK_WifiAddOpenNetwork(const char *ssid, int ch)
{
    __syscall_wifi_addopennetwork_params p;
    p.ssid = ssid;
    p.ch = ch;
    return deferred_call(__syscall_wifi_addopennetwork, &p);
}

int GK_WifiAddPSKNetwork(const char *ssid, int ch, const char *psk)
{
    __syscall_wifi_addpsknetwork_params p;
    p.ssid = ssid;
    p.ch = ch;
    p.psk = psk;
    return deferred_call(__syscall_wifi_addpsknetwork, &p);
}
