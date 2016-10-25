#ifndef __t11_debug_h__
#define __t11_debug_h__

#include "MICO.h"
#define debug(format, ...)  printf("[%s %s:%d][%d]" format "\r\n",SHORT_FILE,__func__,__LINE__, mico_get_time(), ##__VA_ARGS__)

#define t11_print_mem(addr,len) do{\
    for(int line =0;line<=len/16;line++){\
        printf("%p :", ((char*)addr)+line*16);\
        for(int col = 0; col<16&&line*16+col<len;col++){\
            printf(" %02x",((char*)addr)[line*16+col]);\
        }\
        printf("\r\n");\
    }\
}while(0)


#endif