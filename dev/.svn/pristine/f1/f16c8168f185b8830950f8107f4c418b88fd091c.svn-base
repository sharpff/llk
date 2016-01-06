#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <wm_os.h>
#include <app_framework.h>
#include <wmtime.h>
#include <partition.h>
#include <cli.h>
#include <cli_utils.h>
#include <wlan.h>
#include <psm.h>
#include <wmstdio.h>
#include <wmsysinfo.h>
#include <wm_net.h>
#include <httpd.h>
#include <wifidirectutl.h>
#include <wmlog.h>



#ifdef DEBUG_APP
#define APPLOGE(...)             \
    wmlog_e("APPLOG", ##__VA_ARGS__)
#define APPLOGW(...)             \
    wmlog_w("APPLOG", ##__VA_ARGS__)
#define APPLOG(...)             \
    wmlog("APPLOG", ##__VA_ARGS__)
#else
#define APPLOGE(...)
#define APPLOGW(...)
#define APPLOG(...)
#endif /* ! DEBUG_APP */

#endif