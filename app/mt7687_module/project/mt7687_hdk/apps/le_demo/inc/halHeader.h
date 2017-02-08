#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdio.h>
#include "FreeRTOS.h"
#include "sockets.h"
#include "task.h"
#include "queue.h"
#include "nvdm.h"

//#define HW_AES

// #define DEBUG_APP
// #ifdef DEBUG_APP
// #define applog(_mod_name_, _fmt_, ...) \
//     { \
//         const char * p = strrchr(__FILE__, '/'); \
//         printf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : __FILE__, __LINE__); \
//     }

// #define APPLOG(...) \
//     applog("LEAPP", ##__VA_ARGS__)

// #define APPLOGW(...) \
//     applog("LEAPP[W]", ##__VA_ARGS__)

// #define APPLOGE(...) \
//     applog("LEAPP[E]", ##__VA_ARGS__)

// #define APPPRINTF(...) \
//     printf(__VA_ARGS__)
// #else
// #define APPLOGE(...)
// #define APPLOGW(...)
// #define APPLOG(...)
// #define APPPRINTF(...)
// #endif /* ! DEBUG_APP */

void* hal_memset(void *buf, int i, unsigned int len);
void* hal_memcpy(void *dst, const void *src, unsigned int len);
int hal_memcmp(const void *buf1, const void *buf2, unsigned int len);
unsigned int  hal_strlen(const char *str);
int hal_strcmp(const char *dst, const char *src);
int hal_strncmp(const char *dst, const char *src, unsigned int len);
char * hal_strcpy(char *dst, const char *src);
long int hal_strtol(const char *str, char **c, int adecimal);
char *hal_strstr(const char *haystack, const char *needle);
char *hal_strrstr(const char *haystack, const char *needle);
int hal_sprintf(char *buf, const char *fmt, ...);
int hal_snprintf(char *buf, size_t size, const char *fmt, ...);
int hal_vsnprintf(char *buffer, size_t n, const char *format, va_list ap);
int hal_strcoll(const char *l, const char *r);
void hal_abort();

int leLedRead(uint8_t *data, int* dataLen);
int leLedWrite(const uint8_t *data, int dataLen);
void leLedBlueFastBlink(void);
void leLedBlueSlowBlink(void);
void leLedSetDefault(void);
void leLedReset(void);
void leLedInit(void);
int leGetConfigMode(void);
int leSetConfigMode(uint8_t mode);
#endif
