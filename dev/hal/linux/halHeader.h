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



#define APPLOG(_fmt_, ...)\
    printf("LE "_fmt_, ##__VA_ARGS__)

#define APPLOGW(_fmt_, ...)\
    printf("LE [W]"_fmt_, ##__VA_ARGS__)
    
#define APPLOGE(_fmt_, ...)\
    printf("LE [E]"_fmt_, ##__VA_ARGS__)

#define APPPRINTF(...) \
    printf(__VA_ARGS__)

#define delayMS(ms) \
    usleep(ms*1000)

// test only
#define SELF_IP "192.168.3.109"
// #define UUID_BEING_CTRL "10000100091000610006ffffffffffff"
#define UUID_BEING_CTRL "10000100101000010007f0b429000012"

// #define LOCAL_TEST_IP "10.57.149.117"
// #define LOCAL_TEST_IP "192.168.3.172"
#define LOCAL_TEST_IP "192.168.1.102"
#define LOCAL_TEST_PORT 59673

// #define REMOTE_IP "10.154.252.130"
// #define REMOTE_IP "192.168.31.142"
#define BIND_DEBUG


#endif