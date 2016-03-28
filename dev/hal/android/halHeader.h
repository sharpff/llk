#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <net/if.h>
#include <pthread.h>


#include <android/log.h>
#define LE_ANDROID_LOG_ERROR    1
#define LE_ANDROID_LOG_DEBUG    1
#define LE_ANDROID_LOG_WARN     1
#define APPLOG(__fmt__, ...)  do {if (LE_ANDROID_LOG_DEBUG) {__android_log_print(ANDROID_LOG_DEBUG, TAG_LOG, "[D]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)
#define APPLOGW(__fmt__, ...)  do {if (LE_ANDROID_LOG_WARN) {__android_log_print(ANDROID_LOG_WARN, TAG_LOG, "[W]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)
#define APPLOGE(__fmt__, ...)  do {if (LE_ANDROID_LOG_ERROR)  {__android_log_print(ANDROID_LOG_ERROR, TAG_LOG, "[E]: %s, %s, %d\r\n" __fmt__ "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}}while(0)

#define delayMS(ms) \
    usleep(ms*1000)

// test only
#define SELF_IP "192.168.3.109"


// #define LOCAL_TEST_IP "10.57.149.117"
#define LOCAL_TEST_IP "192.168.3.109"
#define LOCAL_PORT 59673

// #define REMOTE_IP "10.154.252.130"
// #define REMOTE_IP "192.168.31.142"
#define BIND_DEBUG


#endif
