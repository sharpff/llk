#ifndef __HEADER_H__
#define __HEADER_H__

#ifdef __cplusplus
extern "C"
{
#endif
    

#ifdef __MRVL_MW300__
#define PF_VAL 1
// #pragma message("__MRVL_MW300__")
#include <sys/types.h>
#include <wmlog.h>
#include <wm_os.h>
// #include <app_framework.h>
#include <lwip/sockets.h>
#include <wmstdio.h>
#include <wmsysinfo.h> 


#define lelog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = strrchr(__FILE__, '/'); \
        wmprintf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__); \
    }

#define LELOG(...) \
    lelog("LE", ##__VA_ARGS__)

#define LELOGW(...) \
    lelog("LE[W]", ##__VA_ARGS__)

#define LELOGE(...) \
    lelog("LE[E]", ##__VA_ARGS__)

#define LEPRINTF(...) \
    wmprintf(__VA_ARGS__)
        
#define delayms(ms) \
    os_thread_sleep(os_msec_to_ticks(ms)) 


// for json_generator.h
#ifndef int64_t
typedef long long int64_t;
#endif


#elif defined (__ATMEL__)
#define PF_VAL 2
#pragma message("__ATMEL__")

#elif defined (LINUX)
#define PF_VAL 3
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h> 
#include <errno.h>
#include <unistd.h>
#include <string.h>

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int16_t
typedef short int16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#define lelog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = strrchr(__FILE__, '/'); \
        printf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__); \
    }

#define LELOG(...) \
    lelog("LE", ##__VA_ARGS__)

#define LELOGW(...) \
    lelog("LE[W]", ##__VA_ARGS__)

#define LELOGE(...) \
    lelog("LE[E]", ##__VA_ARGS__)

#define LEPRINTF(...) \
    printf(__VA_ARGS__)

#define delayms(ms) \
    usleep(ms*1000)
    
//#define LELOG(...)

#elif defined (__ANDROID__)
#define PF_VAL 4
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h> 
#include <errno.h>
#include <unistd.h>


#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int16_t
typedef short int16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#include <android/log.h>
#define LE_ANDROID_LOG_ERROR    1
#define LE_ANDROID_LOG_DEBUG    1
#define LE_ANDROID_LOG_WARN     1

#define LELOG(__fmt__, ...)  do {if (LE_ANDROID_LOG_DEBUG) {__android_log_print(ANDROID_LOG_DEBUG, TAG_LOG, "[D]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)
#define LELOGW(__fmt__, ...)  do {if (LE_ANDROID_LOG_WARN) {__android_log_print(ANDROID_LOG_WARN, TAG_LOG, "[W]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)
#define LELOGE(__fmt__, ...)  do {if (LE_ANDROID_LOG_ERROR)  {__android_log_print(ANDROID_LOG_ERROR, TAG_LOG, "[E]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)

#define delayms(ms) \
    usleep(ms*1000)

#define LEPRINTF(...)    

#elif defined (WIN32)
#define PF_VAL 5
#include <stdio.h>
#include <string.h>
#include <winsock.h>
#include <stdlib.h>
//#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef int8_t
typedef char int8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int16_t
typedef short int16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#define lelog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = strrchr(__FILE__, '\\'); \
        printf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__); \
    }

#define LELOG(...) \
    lelog("LE", ##__VA_ARGS__)

#define LELOGW(...) \
    lelog("LE[W]", ##__VA_ARGS__)

#define LELOGE(...) \
    lelog("LE[E]", ##__VA_ARGS__)

#define LEPRINTF(...) \
    printf(__VA_ARGS__)


#define delayms(ms) \
    Sleep(ms)

#define inet_ntop(a, b, c, d) inet_ntoa(*(struct in_addr *)b)

//#define LELOG(...)
#else

#define LELOG(...)
#pragma error ("no adpation...")

#endif


// #define LOG_SENGINE
// #define LOG_IO
#define LOG_PROTOCOL
#define LOG_STATE
// #define LOG_PACK
// #define LOG_AIRCONFIG_CTRL

        

#ifdef __cplusplus
}
#endif


#endif /* __HEADER_H__ */
