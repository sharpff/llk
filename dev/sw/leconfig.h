#ifndef __LECONFIG_H__
#define __LECONFIG_H__


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __LE_SDK__
#include "header.h"
#else
#include "halHeader.h"
#endif /* __LE_SDK__ */

#if 0
#define TEST_SDK
#else
#define TEST_DEV
#endif

        
// helper function
// TODO: make sure the exactly macro for android & linux
#if !defined (LINUX) && !defined (ANDROID) && !defined(WIN32)
#define memset hal_memset
#define memcpy hal_memcpy
#define memcmp hal_memcmp
#define strlen hal_strlen
#define strcmp hal_strcmp
#define strncmp hal_strncmp
#define strcpy hal_strcpy
#define strtol hal_strtol
#define strstr hal_strstr
#endif

// osal
#define MUTEX_LOCK halLock(NULL, __FILE__, __LINE__)
#define MUTEX_UNLOCK halUnlock(NULL, __FILE__, __LINE__)



#define AES_LEN (128/8)
#define RSA_LEN (1024/8)
#define RSA_RAW_LEN ((1024/8) - 11)
#define MD5_LEN (16)

#define USED(a) ((void)a)



// #define LOCAL_TEST_IP "10.57.149.117"
#define LOCAL_TEST_IP "192.168.3.109"
#define LOCAL_PORT 59673

// #define REMOTE_IP "10.154.252.130"
// #define REMOTE_IP "192.168.31.142"

#if 0
// vpn out side
// #define REMOTE_IP "192.168.3.107"
// #define REMOTE_PORT 5545
// #define REMOTE_IP "10.58.187.59"
// #define REMOTE_IP "10.58.184.174"
// #define REMOTE_IP "10.58.187.104"
//#define REMOTE_IP "10.58.185.120"

#define REMOTE_IP "115.182.94.173"
#define REMOTE_PORT 5546
#else
// internal test
#define REMOTE_IP "10.58.185.120"
#define REMOTE_PORT 5546 

#endif

#ifdef __cplusplus
}
#endif


#endif /* __LECONFIG_H__ */
