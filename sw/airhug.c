/*
 * airhug.c
 *
 * Create on: 2017-01-19
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#include <stdio.h>
#include <string.h>
#include "leconfig.h"
#include "utility.h"
#include "io.h"
#include "data.h"
#include "airhug.h"

#define SYN_DATA1   (0x46)
#define SYN_DATA2   (0x45)
#define SYN_BASE    (0x02)
#define SYN_OFFSET  (0x49)

#define MAX_WAIT_DEV    8

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
    hugmsg_t msg;
    uint8_t data[sizeof(hugmsg_t) + 1];
} s_hug;


static uint8_t calcrc(uint8_t *ptr, uint32_t len);

void airhug_reset(void)
{
    memset(&s_hug, 0, sizeof(s_hug));
}

int airhug_feed_data(uint8_t *src, uint8_t *dst, uint8_t *bssid, uint32_t datalen)
{
    int i, flag;
    static const uint8_t mulhead[] = { 0x01, 0x00, 0x5E };

    if(dst[0] != mulhead[0] || dst[1] != mulhead[1] || dst[2] != mulhead[2] || datalen > 200) {
        return 0;
    }
    switch(s_hug.state)
    {
        case HUG_STATE_WAIT:
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
                    if(s_hug.devs[i].dst[5] > sizeof(s_hug.data)) { // 太长
                        break;
                    }
                    // 锁定
                    s_hug.crc = dst[5];
                    s_hug.len = s_hug.devs[i].dst[5];
                    s_hug.state = HUG_STATE_LOCK;
                    s_hug.base = s_hug.devs[i].datalen - SYN_BASE;
                    LELOG("Lock! Total %u bytes, base = %u, crc = 0x%02x", s_hug.len, s_hug.base, s_hug.crc);
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
            i = datalen - s_hug.base - SYN_BASE;
            if(i < 0 || (((dst[3] + dst[4])&0xff) != dst[5]) || i >= s_hug.len) {
                break;
            }
            LELOG("idx %3d  %02x %02x %02x %u", i, dst[3], dst[4], dst[5], datalen);
            s_hug.data[i + 0] = dst[3];
            s_hug.data[i + 1] = dst[4];
            if(s_hug.data[0] + 1 > s_hug.len) {
                break;
            }
            if(calcrc(s_hug.data, s_hug.len) == s_hug.crc && s_hug.data[0] != strlen((char *)(&s_hug.data[1]))) {
                memcpy(s_hug.msg.ssid, &s_hug.data[1], s_hug.data[0]);
                memcpy(s_hug.msg.passwd, &s_hug.data[1 + s_hug.data[0]], s_hug.len - s_hug.data[0] - 1);
                s_hug.state = HUG_STATE_GET;
                LELOG("Get ssid = '%s', passwd = '%s'\n", s_hug.msg.ssid, s_hug.msg.passwd);
                {
                    int ret = 0;
                    PrivateCfg cfg;
                    lelinkStorageReadPrivateCfg(&cfg);
                    LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                            cfg.data.nwCfg.config.ssid,
                            cfg.data.nwCfg.config.psk, 
                            cfg.data.nwCfg.configStatus);
                    strcpy(cfg.data.nwCfg.config.ssid, s_hug.msg.ssid);
                    cfg.data.nwCfg.config.ssid_len = strlen(s_hug.msg.ssid);
                    cfg.data.nwCfg.config.ssid[cfg.data.nwCfg.config.ssid_len] = '\0';
                    strcpy(cfg.data.nwCfg.config.psk, s_hug.msg.passwd);
                    cfg.data.nwCfg.config.psk_len = strlen(s_hug.msg.passwd);
                    cfg.data.nwCfg.config.psk[cfg.data.nwCfg.config.psk_len] = '\0';
                    cfg.data.nwCfg.configStatus = 1;
                    ret = lelinkStorageWritePrivateCfg(&cfg);
                    LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
                }
            }
            break;
        case HUG_STATE_GET:
            break;
        default:
            return -1;
    }
    return s_hug.state;
}

int airhug_get(hugmsg_t *msg)
{
    if(s_hug.state == HUG_STATE_GET) {
        memcpy(msg, &s_hug.msg, sizeof(*msg));
        return 0;
    }
    return -1;
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

