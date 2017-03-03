#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdio.h>
#include "FreeRTOS.h"
#include "sockets.h"
#include "task.h"
#include "queue.h"
#include "nvdm.h"


#define DEBUG_APP
#ifdef DEBUG_APP
#define applog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = strrchr(__FILE__, '/'); \
        printf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : __FILE__, __LINE__); \
    }

#define APPLOG(...) \
    applog("LEAPP", ##__VA_ARGS__)

#define APPLOGW(...) \
    applog("LEAPP[W]", ##__VA_ARGS__)

#define APPLOGE(...) \
    applog("LEAPP[E]", ##__VA_ARGS__)

#define APPPRINTF(...) \
    printf(__VA_ARGS__)
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
