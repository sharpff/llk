#include "leconfig.h"
#include "utility.h"
#include "airconfig_ctrl.h"

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
    ctx->address.sin_addr.s_addr = inet_addr("239.101.1.1");
    ctx->address.sin_port = htons((u_short)1234);
    
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
    ctx->address.sin_port = htons((u_short)1234);
    
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
	                LELOG("sendto [%s:%d] [%d][0x%02x] [0x%02x]", ip, port, ret, ret, ret-gin_base);
                }
            }
        }break;
    }
    return ret;
}

static int inner_airconfig_do_config_sync(airconfig_ctx_t *ctx) {
    // int time = 0x7fffffff; //300 * 14 * 2; // 300ms X 14 channel X twice
#ifdef DEBUG_AIR_CONFIG
     int time = 300; // 300ms X 14 channel X twice
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
        delayms(ctx->delay);
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
    int len = 8; // magic & prefix 
    uint16_t data = 0;
    uint8_t ldata = 0;
    uint8_t hdata = 0;
    uint8_t passwd_len = strlen(ctx->passwd);
    uint8_t passwd_crc = 0x7F & crc8((uint8_t *)ctx->passwd, passwd_len);
    // uint8_t ssid_len = strlen(ctx->ssid) < 16 ? 128 + strlen(ctx->ssid) : strlen(ctx->ssid);
    uint8_t ssid_len = strlen(ctx->ssid);
    // TODO: why ssid len is the total  in airkiss?
#define AIRKISS
#ifdef AIRKISS
    ssid_len += passwd_len + 1;
#endif
    uint8_t ssid_crc = 0x7F & crc8((uint8_t *)ctx->ssid, ssid_len);

    for (i = 0; i < repeat; i++) {
        data = 0;
        for (j = 0; j < len; j++) {
            switch (j) {
            case 0: { // ssid length
                ldata = (ssid_len >> 4) & 0x0F;
            }break;
            case 1: {
                ldata = ssid_len & 0x0F;
            }break;
            case 2: {// ssid crc
                ldata = (ssid_crc >> 4) & 0x0F;
            }break;
            case 3: {
                ldata = ssid_crc & 0x0F;
            }break;
            case 4: {// passwd length
                ldata = (passwd_len >> 4) & 0x0F;
            }break;
            case 5: {
                ldata = passwd_len & 0x0F;
            }break;
            case 6: {// passwd crc
                ldata = (passwd_crc >> 4) & 0x0F;
            }break;
            case 7: {
                ldata = passwd_crc & 0x0F;
            }break;
            default:
                break;
            }
            hdata = j << 4;
            data = hdata | ldata;

            inner_airconfig_sendto(ctx, gin_base + data);
            delayms(ctx->delay);
        }
    }

    return 1;
}

static int inner_airconfig_do_config_data(airconfig_ctx_t *ctx) {
#ifdef DEBUG_AIR_CONFIG
    int repeat = 1;
#else
    int repeat = 12;
#endif
    int passwd_len = strlen(ctx->passwd);
    int ssid_len = strlen(ctx->ssid);
    int total_bytes = passwd_len + 1 + ssid_len;
    int total_blocks = (total_bytes - 1)/4 + 1;
    int last_bytes = total_bytes%4 ? total_bytes%4 : 4;
    int i, j, tmp;
    uint8_t buf[32 + 32 + 1] = {0};
    if (!ctx) {
        return 0;
    }

    memcpy(buf, ctx->passwd, passwd_len);
    
    // encrypt
    // for (i = 0; i < passwd_len; i++) {

    // }

    buf[passwd_len] = '$'; // TODO: rand it
    memcpy(buf + passwd_len + 1, ctx->ssid, ssid_len);
    while (repeat--) {
        for (i = 0; i < total_blocks; i++) {
            int bytes = (i+1) == total_blocks ? last_bytes : 4;
            uint16_t data = 0;
            uint8_t tmp_buf[5] = { 0 };
            tmp_buf[0] = (uint8_t)i;
            memcpy(&tmp_buf[1], &buf[i*4], bytes);
            tmp = 1;
            while (tmp--) {
                //data = 0x0080 | (crc8((uint8_t*)&i, 1) + crc8(buf + i*bytes, bytes));
                data = 0x0080 | crc8((uint8_t *)tmp_buf, bytes + 1);
                inner_airconfig_sendto(ctx, gin_base + data); // crc 
                delayms(ctx->delay);
                inner_airconfig_sendto(ctx, (gin_base + i) | 0x0080); // seq id
                for (j = 0; j < bytes; j++) {
                    inner_airconfig_sendto(ctx, (gin_base + buf[i*4 + j]) | 0x0100);
                    delayms(ctx->delay);
                }
            }
        }
    }

    return 1;
}

void *airconfig_new(const char *param) {
    int type = 0, sock = -1;
    char str_aes[33] = { 0 };
    
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
    LELOG("sscanf ret[%d]", ret);
    LELOG("sscanf SSID[%s]", gin_airconfig_ctx.ssid);
    LELOG("sscanf PASSWD[%s]", gin_airconfig_ctx.passwd);
    LELOG("sscanf AES[%s]", str_aes);
    LELOG("sscanf TYPE[%d]", type);
    LELOG("sscanf DELAY[%d]", gin_airconfig_ctx.delay);
    
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
    
    //int i = 0;
    //LELOG("AES:");
    //for (i = 0; i < AES_128; i++) {
        //LELOG("%02x", ctx->aes[i]);
    //}
    //LELOG("");
    
    while (!inner_airconfig_do_config_sync(ctx))
        ;
    
    while (!inner_airconfig_do_config_head(ctx))
        ;
    
    while (!inner_airconfig_do_config_data(ctx))
        ;
    
    return 1;
}


void airconfig_delete(void *context) {
    memset(&gin_airconfig_ctx, 0, sizeof(gin_airconfig_ctx));
    return;
}

