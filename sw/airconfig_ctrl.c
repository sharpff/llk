#include "leconfig.h"
#include "utility.h"
#include "airconfig_ctrl.h"
#include "protocol.h"

#ifndef LOG_AIRCONFIG_CTRL
#ifdef LELOG
#undef LELOG
#define LELOG(...)
#endif

#ifdef LELOG
#undef LELOGW
#define LELOGW(...)
#endif

// #ifdef LELOG
// #undef LELOGE
// #define LELOGE(...)
// #endif

#ifdef LEPRINTF
#undef LEPRINTF
#define LEPRINTF(...)
#endif
#endif

#define LECONFIG_MCAST_ADDR "239.101.1.1"
#define LECONFIG_PORT 59678

#ifndef EMW3081

typedef struct {
    char ssid[32];
    char passwd[32];
    uint8_t aes[16];
    int sock;
    uint32_t delay;
    struct sockaddr_in address;
}airconfig_ctx_t;

static airconfig_ctx_t gin_airconfig_ctx;
static uint16_t gin_base = 20;

static int inner_new_multicast(airconfig_ctx_t *ctx) {
    int multicastTTL = 255;
    uint8_t mcTTL = (uint8_t)multicastTTL;
	
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        LELOGE("socket() failed!");
        return -1;
    }
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &mcTTL, sizeof(mcTTL)) < 0) {
        LELOGE("setsockopt() failed!");
        return -2;
    }
    
    ctx->address.sin_family = AF_INET;
    ctx->address.sin_addr.s_addr = inet_addr(LECONFIG_MCAST_ADDR);
    ctx->address.sin_port = htons((u_short)LECONFIG_PORT);
    
    return sock;
}

static int inner_new_broadcast(airconfig_ctx_t *ctx) {
    int sock = -1;
    int bBroadcast = 1;
    int ret = 0;
    char br[32] = {0};

    ret = halGetBroadCastAddr(br, sizeof(br));
    if (0 >= ret) {
        LELOGE("halGetBroadCastAddr error");
        return -1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        LELOGE("socket() failed! [%d]", errno);
        return -2;
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *)&bBroadcast, sizeof(bBroadcast)) < 0) {
        LELOGE("setsockopt() failed!");
        return -3;
    }
    
    ctx->address.sin_family = AF_INET;
    ctx->address.sin_addr.s_addr = inet_addr(br);
    // ctx->address.sin_addr.s_addr = inet_addr("192.168.31.164");
    ctx->address.sin_port = htons((u_short)LECONFIG_PORT);
    
    return sock;
}

static int inner_airconfig_sendto(const airconfig_ctx_t *ctx, int data) {
    int ret = 0;
    struct timeval tv;
    fd_set wset;
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
	FD_ZERO(&wset);
    FD_SET(ctx->sock, &wset);
    
    //int optVal = 0;
    //int optLen = sizeof(optVal);
    //getsockopt(ctx->sock, 
        //SOL_SOCKET, 
        //SO_SNDBUF, 
        //(char*)&optVal, 
        //&optLen);
    //LELOG("SNDBUF [%d]", optVal);
    
    switch (select(ctx->sock + 1, 0, &wset, 0, &tv)) {
    case 0: { // timeout
            LELOG("select timeout");
        }break;
    case -1: { // error
            LELOGE("select");
        }break;
    default: {
            if (FD_ISSET(ctx->sock, &wset)) {
                ret = sendto(ctx->sock, (const char *)&data, data, 0, (struct sockaddr*)&(ctx->address), sizeof(ctx->address));
                if (0 > ret) {
                    LELOGE("sendto [%d] errno[%d]", ret, errno);
                } else {
                    //char *ip = inet_ntop(ctx->address.sin_addr);
                    char ip[16] = { 0 };
                    uint16_t port = ntohs(ctx->address.sin_port);
                    inet_ntop(AF_INET, (void *)&ctx->address.sin_addr, ip, sizeof(ip));
                    USED(ip);
                    USED(port);
	                LELOG("sendto [%s:%d] [%d][0x%02x] [0x%02x] [%c]", ip, port, ret, ret, ret-gin_base, ret-gin_base);
                }
            }
        }break;
    }
    return ret;
}

static int inner_airconfig_do_config_sync(airconfig_ctx_t *ctx) {
    // int time = 0x7fffffff; //300 * 14 * 2; // 300ms X 14 channel X twice
#ifdef DEBUG_AIR_CONFIG
     int time = 400; // 300ms X 14 channel X twice
#else
     // int time = 300 * 14 * 2; // 300ms X 14 channel X twice
     int time = 300 * 11 * 2; // 300ms X 14 channel X twice
#endif
    int count = time/ctx->delay;
    
    if (!ctx) {
        return 0;
    }
    while (count--) {
        inner_airconfig_sendto(ctx, gin_base + count % 4 + 1);
        halDelayms(ctx->delay);
    };
    return 1;
}

static int inner_airconfig_do_config_head(airconfig_ctx_t *ctx) {
    if (!ctx) {
        return 0;
    }

    int i = 0, j = 0;
#ifdef DEBUG_AIR_CONFIG
    int repeat = 1; // 
#else
    int repeat = 12; // 
#endif
    int len = 4; // magic & prefix 
    uint8_t buff[4];
    uint16_t data = 0;
    uint8_t ldata = 0;
    uint8_t hdata = 0;
    buff[3] = strlen(ctx->passwd);
    buff[2] = strlen(ctx->ssid);
    buff[1] = crc8((uint8_t *)ctx->ssid, buff[2]);
    buff[0] = crc8((uint8_t *)&buff[1], 3);
    for (i = 0; i < repeat; i++) {
        for (j = 0; j < len; j++) {
            ldata = ((buff[j] >> 4) & 0x0F);
            hdata = ((2*j) << 4);
            data = hdata | ldata;
            inner_airconfig_sendto(ctx, gin_base + data);
            halDelayms(ctx->delay);
            ldata = (buff[j] & 0x0F);
            hdata = ((2*j+1) << 4);
            data = hdata | ldata;
            inner_airconfig_sendto(ctx, gin_base + data);
            halDelayms(ctx->delay);
        }
    }
    return 1;
}

void inner_airconfig_str2bin(uint8_t* src, uint16_t src_len, uint8_t* dst, uint16_t* dst_len) {
    uint16_t i, temp_len;
    uint8_t half_data;

    temp_len = (src_len*8)/3;

    if((src_len*8)%3 > 0) {
      temp_len++;
    }

    for(i=0; i<temp_len; i++) {
      uint8_t a = (i*3%8);
      uint8_t b = 8-a;
      uint8_t c = i*3/8;

      if(c >= src_len) {
        APPLOGE("lelink_str2bin len error len =[%d]\r\n ", c);
      }

      dst[i] = ((src[c] << a) & 0xE0);
      dst[i] = (dst[i] >> 5);
      if(a != 0 && c+1 < src_len && b<3 ) {
        if(b == 2) {
          half_data = (src[c+1] & 0x80);
          dst[i] |= (half_data >> 7);
        } else {
          half_data = (src[c+1] & 0xC0);
          dst[i] |= (half_data >> 6);
        }
      }
    }

    *dst_len = temp_len;
}

static int inner_airconfig_do_config_data(airconfig_ctx_t *ctx) {
#ifdef DEBUG_AIR_CONFIG
    int repeat = 1;
#else
    int repeat = 12;
#endif
    int passwd_len = strlen(ctx->passwd);
    int ssid_len = strlen(ctx->ssid);
    int total_bytes = 1 + passwd_len + 1 + ssid_len;
    uint8_t buf[1 + 32 + 32 + 1] = {0};
    uint8_t bin[128] = {0};
    uint16_t bin_len, i;
    if (!ctx) {
        return 0;
    }

    memcpy(&buf[1], ctx->passwd, passwd_len);
    
    (buf + 1)[passwd_len] = 0xFF & genRand();
    memcpy((buf + 1) + passwd_len + 1, ctx->ssid, ssid_len);
    buf[0] = crc8((buf + 1), passwd_len + 1 + ssid_len);
    inner_airconfig_str2bin(buf, passwd_len+ssid_len+2, bin, &bin_len);
    APPLOG("====================> [%d][%d][%d]", passwd_len, bin_len, total_bytes);
    while (repeat--) {
        uint16_t data = 0;
        for (i = 0; i < bin_len; i++) {
            data = (i << 3);
            data |= (bin[i] & 0x07);
            inner_airconfig_sendto(ctx, (gin_base + data));
            //APPLOG("data [%d]====================>", data);
            halDelayms(ctx->delay * 5);
        }
    }
    return 1;
}

void *airconfig_new(const char *param) {
    int type = 0, sock = -1;
    char str_aes[33] = { 0 };
    char br[MAX_IPLEN] = {0};
    int ret = sscanf(param,
        "SSID=%[^,],PASSWD=%[^,],AES=%[^,],TYPE=%d,DELAY=%d", 
        gin_airconfig_ctx.ssid,
        gin_airconfig_ctx.passwd,
        str_aes,
        &type,
        &(gin_airconfig_ctx.delay));
    
    if (AES_LEN * 2 == strlen(str_aes)) {
        if (hexStr2bytes(str_aes, gin_airconfig_ctx.aes, sizeof(gin_airconfig_ctx.aes))) {
            return NULL;
        }
    }
    USED(ret);
    if (0 == strcmp("nill", gin_airconfig_ctx.passwd)) {
        memset(gin_airconfig_ctx.passwd, 0, sizeof(gin_airconfig_ctx.passwd));
    }
    LELOG("sscanf ret[%d]", ret);
    LELOG("sscanf SSID[%s]", gin_airconfig_ctx.ssid);
    LELOG("sscanf PASSWD[%s]", gin_airconfig_ctx.passwd);
    LELOG("sscanf AES[%s]", str_aes);
    LELOG("sscanf TYPE[%d]", type);
    LELOG("sscanf DELAY[%d]", gin_airconfig_ctx.delay);
    
    ret = halGetBroadCastAddr(br, sizeof(br));
    if (0 >= ret) {
        strcpy(br, "255.255.255.255");
    }
    ret = halCastProbing(LECONFIG_MCAST_ADDR, br, LECONFIG_PORT);
    if (0 >= ret) {
        LELOGW("inner_new_multicast halCastProbing [%d]", ret);
        return NULL;
    }

    switch (type) {
    case 1: {
            sock = inner_new_multicast(&gin_airconfig_ctx);
            if (0 > sock) {
                LELOGE("inner_new_multicast failed [%d]", sock);
                return NULL;
            }
            gin_airconfig_ctx.sock = sock;
        }break;
    case 2: {
            sock = inner_new_broadcast(&gin_airconfig_ctx);
            if (0 > sock) {
                LELOGE("inner_new_broadcast failed [%d]", sock);
                return NULL;
            }
            gin_airconfig_ctx.sock = sock;    
        }break;
    default:
        break;
    }
    
    return &gin_airconfig_ctx;
}

int airconfig_do_config(void *context) {
    airconfig_ctx_t *ctx = (airconfig_ctx_t *)context;

    // current ssid + psk len should be <= 46
    // data content is [crc + psk + rand + ssid] = 48 bytes, crc & rand are all 1 byte.
    if ((strlen(ctx->ssid) + strlen(ctx->passwd)) > 46) {
        LELOGE("airconfig_do_config param error ssid len[%d] psk len[%d]", strlen(ctx->ssid), strlen(ctx->passwd));
        return -1;
    }
    //int i = 0;
    //LELOG("AES:");
    //for (i = 0; i < AES_128; i++) {
        //LELOG("%02x", ctx->aes[i]);
    //}
    //LELOG("");
    LELOGE("111111");
    while (!inner_airconfig_do_config_sync(ctx))
        ;
    
    LELOGE("222222");
    while (!inner_airconfig_do_config_head(ctx))
        ;
    
    LELOGE("333333");
    while (!inner_airconfig_do_config_data(ctx))
        ;
    
    return 1;
}


void airconfig_delete(void *context) {
    close(gin_airconfig_ctx.sock);
    memset(&gin_airconfig_ctx, 0, sizeof(gin_airconfig_ctx));
    return;
}

#endif
