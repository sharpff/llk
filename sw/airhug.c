/*
 * airhug.c
 *
 * Create on: 2017-01-19
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

// #include <stdio.h>
// #include <string.h>
#include "airhug.h"
#include "rc4Wrapper.h"

#define AIRHUG_VERSION      (0x1)
#define AIRHUG_MAX_LEN      (255)

#define SYN_DATA1   (0x46)
#define SYN_DATA2   (0x5a)
#define SYN_OFFSET  (0x08)

#define MAX_WAIT_DEV        8

#define LOG_FILE    0

typedef enum {
    HUG_STATE_WAIT = 0,
    HUG_STATE_LOCK = 1,
    HUG_STATE_GET = 2,
} HUG_STATE_t;

static struct {
    HUG_STATE_t state;
    int devnum;
    int lastidx;
    struct {
        uint8_t src[6];
        uint8_t dst[6];
        uint8_t datalen;
    } devs[MAX_WAIT_DEV];
    uint32_t len;
    uint32_t base;
    uint8_t crc;
    uint8_t data[AIRHUG_MAX_LEN];
} s_hug;

static void printmac(const uint8_t *mac);
static uint8_t calcrc(uint8_t *ptr, uint32_t len);

/*
 * 功能: 得到数据
 *
 * 参数: 
 *       buf : 返回数据缓冲区
 *      size : data 空间大小(目前最大支持255)
 *
 * 返回值:
 *      -1 : 出错
 *    其它 : 得到数据长度
 *
 */
static int airhug_get_raw(uint8_t *buf, uint16_t size);

void airhug_reset(void)
{
    memset(&s_hug, 0, sizeof(s_hug));
}

int airhug_feed_data(const uint8_t *src, const uint8_t *dst, const uint8_t *bssid, uint32_t datalen)
{
    int i, flag;
    static const uint8_t mulhead[] = { 0x01, 0x00, 0x5E };
#if LOG_FILE 
    static FILE *fp = NULL;
    if(!fp) {
        fp = fopen("log.txt", "wb");
    }
#endif
    if(dst[0] != mulhead[0] || dst[1] != mulhead[1] || dst[2] != mulhead[2] || datalen > 200) {
        return 0;
    }
    switch(s_hug.state)
    {
        case HUG_STATE_WAIT:
            LELOG("SRC "); printmac(src);
            LELOG(" DST "); printmac(dst);
            LELOG(" BSSID "); printmac(bssid);
            LELOG(" Len %3d", datalen);
            if(dst[3] != SYN_DATA1 || dst[4] != SYN_DATA2) {
                break;
            }
            flag = 0;
            for(i = 0; i < MAX_WAIT_DEV; i++) {
                if(s_hug.devs[i].datalen == 0) {
                    continue;
                }
                if(!memcmp(s_hug.devs[i].src, src, sizeof(s_hug.devs[i].src))) {
                    flag = 1;
                    if(datalen - s_hug.devs[i].datalen != SYN_OFFSET) { // 更新
                        memcpy(s_hug.devs[i].dst, dst, sizeof(s_hug.devs[i].dst));
                        s_hug.devs[i].datalen = datalen;
                        break;
                    }
                    if(s_hug.devs[i].dst[5] > AIRHUG_MAX_LEN) { // 太长
                        break;
                    }
                    // 锁定
                    s_hug.crc = dst[5];
                    s_hug.len = s_hug.devs[i].dst[5];
                    s_hug.state = HUG_STATE_LOCK;
                    s_hug.base = s_hug.devs[i].datalen;
                    LELOG("Lock "); printmac(src);
                    LELOG("Total %u bytes, base = %u, crc = 0x%02x", s_hug.len, s_hug.base, s_hug.crc);
#if LOG_FILE 
                    fprintf(fp, "\nTotal %u bytes, base = %u, crc = 0x%02x\n", s_hug.len, s_hug.base, s_hug.crc);
#endif
                    break;
                }
            }
            if(!flag) {
                if(s_hug.devnum < MAX_WAIT_DEV) {
                    s_hug.devnum++;
                }
                s_hug.lastidx = (s_hug.lastidx + 1) % MAX_WAIT_DEV;
                s_hug.devs[s_hug.lastidx].datalen = datalen;
                memcpy(s_hug.devs[s_hug.lastidx].src, src, sizeof(s_hug.devs[s_hug.lastidx].src));
                memcpy(s_hug.devs[s_hug.lastidx].dst, dst, sizeof(s_hug.devs[s_hug.lastidx].dst));
            }
            break;
        case HUG_STATE_LOCK:
#if 0
            LELOG("SRC "); printmac(src);
            LELOG(" DST "); printmac(dst);
            LELOG(" BSSID "); printmac(bssid);
            LELOG(" Len %3d", datalen);
#endif
            i = datalen - s_hug.base;
            if(i < 0 || (((dst[4] + dst[5]) & 0x7f) != dst[3]) || i >= s_hug.len) {
                break;
            }
#if LOG_FILE 
            fprintf(fp, "idx %3d  %02x %02x %02x %u", i, dst[3], dst[4], dst[5], datalen);
#endif
            s_hug.data[i + 0] = dst[4];
            s_hug.data[i + 1] = dst[5];
            LELOG("idx %3d  %02x %02x %02x %u", i, dst[3], dst[4], dst[5], datalen);
            // LELOG("idx %3d  %02x %02x %02x %u, crc[%02x | %02x] s_hug.len[%d]", i, dst[3], dst[4], dst[5], datalen, 
            //     calcrc(s_hug.data, s_hug.len), s_hug.crc, s_hug.len);
            if(calcrc(s_hug.data, s_hug.len) == s_hug.crc) {
                s_hug.state = HUG_STATE_GET;
                LELOG("Get!!! len = %u", s_hug.len);
#if LOG_FILE 
                fprintf(fp, "Get!!! len = %u", s_hug.len);
#endif
            }
            break;
        case HUG_STATE_GET:
            break;
        default:
            return -1;
    }
    return s_hug.state;
}

static int airhug_get_raw(uint8_t *buf, uint16_t size)
{
    if(!buf) {
        LELOG("airhug_get_raw buf is NULL");
        return -1;
    }
    if(s_hug.state != HUG_STATE_GET) {
        LELOG("airhug_get_raw s_hug.state[%d]", s_hug.state);
        return -1;
    }
    if(s_hug.len > size) {
        LELOG("airhug_get_raw s_hug.len[%d] size[%d]", s_hug.len, size);
        return -1;
    }
    memcpy(buf, s_hug.data, s_hug.len);
    LELOG("airhug_get_raw s_hug.len[%d]", s_hug.len);
    return s_hug.len;
}

/*
 *   -------------------------------------------------------------
 *  |   ver  |  len1  |     ssid    | random |  len2  |   passwd  |
 *   -------------------------------------------------------------
 *  | 1 byte | 1 byte |  len1 bytes | 1 byte | 1 byte | len2 bytes|
 *   -------------------------------------------------------------
 */
int airhug_get(char *ssid, uint16_t ssidlen, char *passwd, uint16_t passwdlen)
{
    int ret;
    uint16_t len = 0;
    uint8_t len1, len2, s[256];
    uint8_t data[AIRHUG_MAX_LEN];
	static const char *key = "09n899uh39nhh9q34kk";

    if((ret = airhug_get_raw(data, sizeof(data))) <= 0) {
        LELOGE("airhug_get airhug_get_raw [%d]", ret);
        return -1;
    }
    len = ret;
    // LELOGE("<======== len[%d]", len);
    // {
    //     int kk = 0;
    //     for (kk = 0; kk < len; kk++) {
    //         if (0 == (kk)%2) {
    //             LEPRINTF("\n");
    //         }
    //         LEPRINTF("%02x ", data[kk]);
    //     }
    //     LEPRINTF("\n");
    // }
    // rc4
    rc4_init(s, (uint8_t *)key, strlen(key));
    rc4_crypt(s, data, len);
    // LEPRINTF("=============\n");
    // {
    //     int kk = 0;
    //     for (kk = 0; kk < len; kk++) {
    //         if (0 == (kk)%2) {
    //             LEPRINTF("\n");
    //         }
    //         LEPRINTF("%02x ", data[kk]);
    //     }
    //     LEPRINTF("\n");
    // }
    // version
    if(data[0] != AIRHUG_VERSION) {
        LELOGE("airhug version error, %02x", data[0]);
        return -2;
    }
    // len
    if(len < 4 || ssidlen + passwdlen - 2 < len - 4) {
        LELOGE("airhug len error, %02x", len);
        return -3;
    }
    // ssid
    len1 = data[1];
    if(len1 < 1 || len1 > ssidlen - 1) {
        LELOGE("airhug ssid len error, %u %u", len1, ssidlen);
        return -4;
    }
    strncpy(ssid, (char *)&data[2], len1);
    // random
    // passwd 
    len2 = data[3 + len1];
    if(len2 < 1 || len2 > passwdlen - 1) {
        LELOGE("airhug passwd len error, %u %u", len2, passwdlen);
        return -5;
    }
    strncpy(passwd, (char *)&data[3 + len1 + 1], len2);
    ssid[len1] = passwd[len2] = '\0';
    LELOG("airhug Get: ssid'%s', passwd'%s'", ssid, passwd);
    return 0;
}

static void printmac(const uint8_t *mac)
{
    LELOG("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],mac[3],mac[4],mac[5]);
}

static uint8_t calcrc(uint8_t *ptr, uint32_t len)
{
    uint8_t i;
    uint8_t crc = 0;

    while(len--)
    {
        crc ^= *ptr++;
        for(i = 0;i < 8;i++) {
            if(crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else  {
                crc >>= 1;
            }
        }
    }
    return crc;
}

