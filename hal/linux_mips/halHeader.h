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

#define LOCAL_TEST_PORT 59673

#define BIND_DEBUG


#endif
