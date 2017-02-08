#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t11_debug.h" 

// #define applog(_mod_name_, _fmt_, ...) \
//     { \
//         const char * __p__ = strrchr(__FILE__, '/'); \
//         printf("[%s] "_fmt_" @%s:%d\r\n", _mod_name_, ##__VA_ARGS__, __p__ ? (__p__ + 1) : "none", __LINE__); \
//     }

// #define APPLOG(...) \
//     applog("LEAPP", ##__VA_ARGS__)

// #define APPLOGW(...) \
//     applog("LEAPP[W]", ##__VA_ARGS__)

// #define APPLOGE(...) \
//     applog("LEAPP[E]", ##__VA_ARGS__)

// #define LEPRINTF(...) \
//     printf(__VA_ARGS__)

#define delayMS(ms) \
    usleep(ms*1000)

#define LOCAL_TEST_PORT 59673

#define BIND_DEBUG


#endif
