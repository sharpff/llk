#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdarg.h>
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


#define DEBUG_APP
#ifdef DEBUG_APP
#define APPLOGE(...)             \
    wmlog_e("APPLOG", ##__VA_ARGS__)
#define APPLOGW(...)             \
    wmlog_w("APPLOG", ##__VA_ARGS__)
#define APPLOG(...)             \
    wmlog("APPLOG", ##__VA_ARGS__)
#define APPPRINTF(...) \
    wmprintf(__VA_ARGS__)
#else
#define APPLOGE(...)
#define APPLOGW(...)
#define APPLOG(...)
#define APPPRINTF(...)
#endif /* ! DEBUG_APP */

inline void* hal_memset(void *buf, int i, unsigned int len);
inline void* hal_memcpy(void *dst, const void *src, unsigned int len);
inline int hal_memcmp(const void *buf1, const void *buf2, unsigned int len);
inline unsigned int  hal_strlen(const char *str);
inline int hal_strcmp(const char *dst, const char *src);
inline int hal_strncmp(const char *dst, const char *src, unsigned int len);
inline char * hal_strcpy(char *dst, const char *src);
inline long int hal_strtol(const char *str, char **c, int adecimal);
inline char *hal_strstr(const char *haystack, const char *needle);
inline int hal_sprintf(char *buf, const char *fmt, ...);
inline int hal_snprintf(char *buf, size_t size, const char *fmt, ...);
inline int hal_vsnprintf(char *buffer, size_t n, const char *format, va_list ap);
inline int hal_strcoll(const char *l, const char *r);
inline void hal_abort();

#endif
