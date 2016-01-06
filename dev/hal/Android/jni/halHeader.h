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


#define APPLOG(_fmt_, ...)\
    printf("LE "_fmt_, ##__VA_ARGS__)

#define APPLOGW(_fmt_, ...)\
    printf("LE [W]"_fmt_, ##__VA_ARGS__)
    
#define APPLOGE(_fmt_, ...)\
    printf("LE [E]"_fmt_, ##__VA_ARGS__)

#define APPPRINTF(...) \
    printf(__VA_ARGS__)


#endif