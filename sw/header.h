#ifndef __HEADER_H__
#define __HEADER_H__

#ifdef __cplusplus
extern "C"
{
#endif
    

#if defined (mw300)
// #define PF_VAL 1
// #pragma message("mw300")

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

#ifndef size_t
typedef unsigned int size_t;
#endif

#ifndef uint64_t
typedef unsigned long long int  uint64_t;
#endif

#ifndef int64_t
typedef long long int64_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

        
// #define delayms(ms) \
//     os_thread_sleep(os_msec_to_ticks(ms)) 


#elif defined (__ATMEL__)
#define PF_VAL 2
#pragma message("__ATMEL__")

#elif defined (LINUX)
// #define PF_VAL 3
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h> 
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// #ifndef uint8_t
// typedef unsigned char uint8_t;
// #endif

// #ifndef uint16_t
// typedef unsigned short uint16_t;
// #endif

// #ifndef int16_t
// typedef short int16_t;
// #endif

// #ifndef uint32_t
// typedef unsigned int uint32_t;
// #endif

// #define lelog(_mod_name_, _fmt_, ...) \
//     { \
//         const char * p = (const char *)strrchr(__FILE__, '/'); \
//         printOut("[%s] "_fmt_" @%s:%d#%u\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__, halGetTimeStamp()); \
//     }

// #define LELOG(...) \
//     lelog("LE", ##__VA_ARGS__)

// #define LELOGW(...) \
//     lelog("LE[W]", ##__VA_ARGS__)

// #define LELOGE(...) \
//     lelog("LE[E]", ##__VA_ARGS__)

// #define LEPRINTF(...) \
//     printf(__VA_ARGS__)

// #define delayms(ms) \
//     usleep(ms*1000)
    
//#define LELOG(...)

#elif defined (__ANDROID__)
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h> 
#include <errno.h>
#include <unistd.h>

extern int printOut(const char *fmt, ...);

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

// #define lelog(_mod_name_, _fmt_, ...) \
//     { \
//         const char * p = strrchr(__FILE__, '\\'); \
//         printOut("[%s] "_fmt_" @%s:%d#%u\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__, halGetTimeStamp()); \
//     }

// #define LELOG(...) \
//     lelog("LE", ##__VA_ARGS__)

// #define LELOGW(...) \
//     lelog("LE[W]", ##__VA_ARGS__)

// #define LELOGE(...) \
//     lelog("LE[E]", ##__VA_ARGS__)

// #define LEPRINTF(...) \
//     printf(__VA_ARGS__)


// #define delayms(ms) \
//     Sleep(ms)

#define inet_ntop(a, b, c, d) inet_ntoa(*(struct in_addr *)b)

//#define LELOG(...)
#elif defined (MT7687)
// #define PF_VAL 6
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

// #ifndef uint8_t
// typedef unsigned char uint8_t;
// #endif

// #ifndef uint16_t
// typedef unsigned short uint16_t;
// #endif

// #ifndef int16_t
// typedef short int16_t;
// #endif

// #define lelog(_mod_name_, _fmt_, ...) \
//     { \
//         const char * p = strrchr(__FILE__, '/'); \
//         printOut("[%s] "_fmt_" @%s:%d#%u\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__, halGetTimeStamp()); \
//     }

// #define LELOG(...) \
//     lelog("LE", ##__VA_ARGS__)

// #define LELOGW(...) \
//     lelog("LE[W]", ##__VA_ARGS__)

// #define LELOGE(...) \
//     lelog("LE[E]", ##__VA_ARGS__)

// #define LEPRINTF(...) \
//     printf(__VA_ARGS__)

// #define delayms(ms) \
//     usleep(ms*1000)
    
//#define LELOG(...)
#elif defined (EWM3081)
//#define PF_VAL 7
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#define lelog(_mod_name_, _fmt_, ...) \
    //{ \
        //const char * p = strrchr(__FILE__, '/'); \
        //printOut("[%s] "_fmt_" @%s:%d#%u\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__, halGetTimeStamp()); \
    //}

//#define LELOG(...) \
    //lelog("LE", ##__VA_ARGS__)

//#define LELOGW(...) \
    //lelog("LE[W]", ##__VA_ARGS__)

//#define LELOGE(...) \
    //lelog("LE[E]", ##__VA_ARGS__)

//#define LEPRINTF(...) \
    //printOut(__VA_ARGS__)

//#define delayms(ms) \
    //usleep(ms*1000)
    
#else

#define LELOG(...)
#pragma error ("no adpation...")

#endif


#define LOG_SENGINE
#define LOG_IO
#define LOG_PROTOCOL
#define LOG_STATE
#define LOG_PACK
// #define LOG_AIRCONFIG_CTRL


        

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define TIMEOUT_SECS_BEGIN(secs) { \
    static uint32_t ot; \
    if ((halGetTimeStamp() - ot) > secs) {

#define TIMEOUT_SECS_END \
    ot = halGetTimeStamp(); \
    }}


#define lelog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = (const char *)strrchr(__FILE__, '/'); \
        printOut("[%s] "_fmt_" @%s:%d#%u\r\n", _mod_name_, ##__VA_ARGS__, p ? (p + 1) : "none", __LINE__, halGetTimeStamp()); \
    }

#define LELOG(...) \
    lelog("LE", ##__VA_ARGS__)

#define LELOGW(...) \
    lelog("LE[W]", ##__VA_ARGS__)

#define LELOGE(...) \
    lelog("LE[E]", ##__VA_ARGS__)

#define LEPRINTF(...) \
    printOut(__VA_ARGS__)


#if !defined (LINUX) && !defined (__ANDROID__) && !defined(WIN32)
#define memset hal_memset
#define memcpy hal_memcpy
#define memcmp hal_memcmp
#define strlen hal_strlen
#define strcmp hal_strcmp
#define strncmp hal_strncmp
#define strcpy hal_strcpy
#define strtol hal_strtol
#define strstr hal_strstr
#define sprintf hal_sprintf
#define snprintf hal_snprintf
#define vsnprintf hal_vsnprintf
#define strcoll hal_strcoll
#define abort hal_abort
#define malloc halMalloc
#define calloc halCalloc
#define realloc halRealloc
#define free halFree
#endif

// halIO
void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl);
int halUartClose(void *dev);
int halUartRead(void *dev, uint8_t *buf, uint32_t len);
int halUartWrite(void *dev, const uint8_t *buf, uint32_t len);
void *halGPIOInit(void);
int halGPIOClose(void *dev);
int halGPIOOpen(int8_t id, int8_t dir, int8_t mode);
int halGPIORead(void *dev, int gpioId, int *val);
int halGPIOWrite(void *dev, int gpioId, const int val);
int halFlashInit(void);
int halFlashDeinit(void);
void *halFlashOpen(void);
int halFlashClose(void *dev);
int halFlashErase(void *dev, uint32_t startAddr, uint32_t size);
int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr);
int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr);
int halGetMac(uint8_t *mac, int len);
void *halPipeOpen(char *name);
int halPipeClose(void *dev);
int halPipeRead(void *dev, uint8_t *buf, uint32_t len);
int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len);

// halOS
int halLockInit(void);
void halDeLockInit(void);
int halLock(void);
int halUnlock(void);
unsigned int halGetTimeStamp(void);
unsigned int halGetUTC(void);
void *halMalloc(size_t size);
void *halCalloc(size_t n, size_t size);
void *halRealloc(void *ptr, size_t size);
void halFree(void *ptr);
int halReboot();



#ifdef __cplusplus
}
#endif


#endif /* __HEADER_H__ */
