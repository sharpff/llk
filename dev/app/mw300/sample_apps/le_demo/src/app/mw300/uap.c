#include <wm_os.h>
#include <wmstdio.h>
#include <wmsysinfo.h>
#include <wmsysinfo.h>
#include <wlan.h>
#include <string.h>
#include <app_framework.h>
#include "uap.h"

#include <wmlog.h>
//#if APPCONFIG_DEBUG_ENABLE
#define dbg(_fmt_, ...)             \
    wmprintf("[appln] "_fmt_"\r\n", ##__VA_ARGS__)
/*#else*/
/*#define dbg(...)*/
/*#endif [> APPCONFIG_DEBUG_ENABLE <]*/

int wlanUAPInit(const char *uuid)
{
    char ssid[32];
    char wpa2_passphrase[32] = "00000000";

    snprintf(ssid, sizeof(ssid), "-lelink0.1-%s", uuid);
    return app_uap_start_with_dhcp(ssid, wpa2_passphrase);
}

void eventUAPStarted(void *data)
{
    dbg("uap interface started");
}

void eventUAPStopped(void *data)
{
    dbg("uap interface stopped");
}

