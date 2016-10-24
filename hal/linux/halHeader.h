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

#define delayMS(ms) \
    usleep(ms*1000)

// test only
#define SELF_IP "192.168.3.100"
// #define SELF_IP "192.168.1.133"
//#define SELF_IP "192.168.67.19"
// #define SELF_IP "192.168.1.113"
// #define SELF_IP "192.168.3.215"

#if 0
#define DOOYA
// #define HONYAR
// #define DINGDING
// #define MYLOCAL

#ifdef DOOYA
#define UUID_BEING_CTRL "10000100091000610006C80E77ABCD40" // dooya1
#define LOCAL_TEST_IP "192.168.3.238" // dooya1
#elif defined HONYAR
#define UUID_BEING_CTRL "10000100101000010007C80E77ABCD50" // honyar1
#define LOCAL_TEST_IP "192.168.3.104" // hoyar1
// #define LOCAL_TEST_IP "192.168.1.114" // hoyar1
#elif defined DINGDING
#define UUID_BEING_CTRL "10000100111000810008C80E77ABCD60" // dingding1
#define LOCAL_TEST_IP "192.168.3.120" // dingding1
#elif defined MYLOCAL
#define UUID_BEING_CTRL "10000100111000810008C80E77ABCDFF" // mylocal
#define LOCAL_TEST_IP "192.168.3.129" // mylocal
#else
#pragma error "no matched"
#endif
// #define UUID_BEING_OTA "10000100101000010007FFFFFFFFFFFF"
#endif


    
#define LOCAL_TEST_PORT 59673

#define BIND_DEBUG


#endif
