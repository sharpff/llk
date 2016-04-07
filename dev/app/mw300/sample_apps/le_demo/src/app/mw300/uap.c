#include <string.h>
#include <wlan.h>
#include <wm_os.h>
#include <wmstdio.h>
#include <wmsysinfo.h>
#include <wmsysinfo.h>
#include <app_framework.h>
#include <lelink/sw/leconfig.h>
#include <lelink/sw/network.h>
#include <lelink/sw/header.h>
#include "uap.h"

static os_thread_t thread_uapconfig;
static os_thread_stack_define(thread_stack_uapconfig, 1024 * 10);
static void thread_uapconfig_proc(os_thread_arg_t thandle);

#define WIFICONFIG_MAGIC    (0x7689)
#define WIFICONFIG_VERSION  (1)

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t checksum;
    uint8_t ssid[32];
    uint8_t wap2passwd[32];
} wificonfig_t;

int wlanUAPInit(const char *uuid)
{
    char ssid[32];
    char wpa2_passphrase[32] = "00000000";

    snprintf(ssid, sizeof(ssid), "-lelink0.1-%s", uuid);
    return app_uap_start_with_dhcp(ssid, wpa2_passphrase);
}

void eventUAPStarted(void *data)
{
    int ret;

    APPLOG("uap interface started");
    ret = os_thread_create(&thread_uapconfig,
        "uapconfig",
        thread_uapconfig_proc,
        (void *)&thread_uapconfig,
        &thread_stack_uapconfig,
        OS_PRIO_3);
    if(ret < 0) {
        APPLOGE("Failed to create thread: thread_uapconfig_proc");
    }
}

void eventUAPStopped(void *data)
{
    APPLOG("uap interface stopped");
}

static void thread_uapconfig_proc(os_thread_arg_t thandle)
{
    int ret;
    uint16_t port;
    char ipaddr[32];
    char buf[UDP_MTU];
    wificonfig_t wc;
    void *ctx  = lelinkNwNew(NULL, 0, 4911, NULL);

    if(!ctx) {
        APPLOGE("New link error");
        goto out;
    }
    while(1) {
        APPLOG("Waitting wifi configure.");
        delayms(1000);
        ret = nwUDPRecvfrom(ctx, (uint8_t *)buf, UDP_MTU, ipaddr, sizeof(ipaddr), &port);
        if(ret > 0 ) {
            APPLOG("nwUDPRecvfrom ret = %d", ret);
            if(ret != sizeof(wc)) {
                APPLOGE("Wrong len = %d", ret);
                continue;
            }
            memcpy(&wc, buf, ret);
            APPLOG("Get ssid[%s] passwd[%s]", wc.ssid, wc.wap2passwd);
        } else {
            APPLOGE("nwUDPRecvfrom ret = %d", ret);
            /*goto out;*/
        }
    }
out:
    if(ctx) {
        lelinkNwDelete(ctx);
    }
    os_thread_self_complete((os_thread_t *)thandle);
}
