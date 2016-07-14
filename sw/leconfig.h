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

#if defined(WIN32) || defined(EWM3801)
#define ALIGNED
#else
#define ALIGNED __attribute__((packed))
#endif

// osal
#define MUTEX_LOCK halLock()
#define MUTEX_UNLOCK halUnlock()


#define AES_LEN (128/8)
#define RSA_LEN (1024/8)
#define RSA_RAW_LEN ((1024/8) - 11)
#define MAX_RSA_PUBKEY (RSA_LEN + RSA_LEN/2)
#define MD5_LEN (16)
#define MAX_UUID (32)
#if (MD5_LEN > AES_LEN)
    #pragma error "MD5_LEN > AES_LEN"
#endif
#define MAX_BUF (1024+256)


#define USED(a) ((void)a)


#define REMOTE_BAK_IP "115.182.94.173"
#define REMOTE_BAK_PORT 5546


#ifdef __cplusplus
}
#endif


#endif /* __LECONFIG_H__ */
