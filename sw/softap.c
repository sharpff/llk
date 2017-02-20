/*
 * softap.c
 *
 * Create on: 2017-02-20
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#include "leconfig.h"
#include "utility.h"
#include "airconfig.h"
#include "io.h"
#include "data.h"
#include "aesWrapper.h"
#include "network.h"
#include "softap.h"

#define WIFICONFIG_LISTEN_PORT      (4911)
#define WIFICONFIG_MAGIC            (0x7689)
#define WIFICONFIG_SOFTAP_VER       (1)
//#define WIFICONFIG_CKSUM_LEN        ((uint32_t)&(((wificonfig_t *)0)->reserved))
#define WIFICONFIG_CKSUM_LEN        (2 + 32 + 32)

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t checksum;
    uint16_t reserved;
    uint8_t ssid[32];
    uint8_t wap2passwd[32];
} wificonfig_t;

int softApStart(void);
int softApCheck(void);
int softApStop(int success);

static void *ginApNetCtx = NULL;
static uint8_t ginSoftAPAESKey[AES_LEN] = {0};

int softApStart(void)
{
    int ret;
    char ssid[32];
    char uuid[32] = {0};
    uint8_t mac[6] = {0};
    uint8_t retCRC8 = 0;
    char wpa2_passphrase[32] = "00000000";
    uint8_t tmpMac[3] = {0};

    softApStop(0);
    if((ret = getTerminalUUID((uint8_t *)uuid, sizeof(uuid))) < 0) {
        LELOGE("getTerminalUUID ret[%d]", ret);
        goto out;
    }
    halGetMac(mac, 6);
    retCRC8 = crc8(mac, sizeof(mac));
    bytes2hexStr(&retCRC8, 1, tmpMac, sizeof(tmpMac));
    snprintf(ssid, sizeof(ssid), "lelink%03d%s", WIFICONFIG_SOFTAP_VER, uuid);
    ssid[29] = tmpMac[0];
    ssid[30] = tmpMac[1];
    LELOG("softApStart crc8[0x%02x] ssid[%s]", retCRC8, ssid);
    memset(ginSoftAPAESKey, 0, AES_LEN);
    if((ret = halSoftApStart(ssid, wpa2_passphrase, ginSoftAPAESKey, AES_LEN))) {
        LELOGE("halSoftApStart ret[%d]", ret);
        goto out;
    }
    ginApNetCtx = lelinkNwNew(NULL, 0, 4911, NULL);
    if(!ginApNetCtx) {
        LELOGE("New link");
        goto out;
    }
    return 0;
out:
    softApStop(0);
    return ret;
}

int softApStop(int success)
{
    halSoftApStop(success);
    if(ginApNetCtx) {
        lelinkNwDelete(ginApNetCtx);
        ginApNetCtx = NULL;
    }
    return 0;
}

int softApCheck(void)
{
    int ret;
    uint8_t sum;
    uint16_t port;
    char ipaddr[32];
    wificonfig_t wc;
    char buf[UDP_MTU];

    if(!ginApNetCtx) {
        return -1;
    }
    LELOG("softApCheck wifi configure...");
    ret = nwUDPRecvfrom(ginApNetCtx, (uint8_t *)buf, UDP_MTU, ipaddr, sizeof(ipaddr), &port);
    if(ret <= 0 ) {
        return -2;
    }
    LELOG("nwUDPRecvfrom ret = %d sizeof(wc)[%d]", ret, sizeof(wc));
    if(ret != ENC_SIZE(AES_LEN, sizeof(wc) + 1)) {
        LELOGE("wrong len = %d", ret);
        return -3;
    }

{
    uint8_t iv[AES_LEN] = { 0 };
    uint8_t key[AES_LEN] = { 0 };
    uint32_t decLen = ENC_SIZE(AES_LEN, sizeof(wc) + 1);
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    if (ginSoftAPAESKey[0]) {
        memcpy(key, ginSoftAPAESKey, AES_LEN);
    } else {
        memcpy(key, getPreSharedToken(), AES_LEN);
    }
    {
        extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
        uint8_t  hexStr[96] = {0};
        bytes2hexStr(key, AES_LEN, hexStr, sizeof(hexStr));
        LELOG("key[%s]", hexStr);
    }

    ret = aes(iv, 
        key, 
        (uint8_t *)&buf,
        &decLen, /* in-len/out-enc size */
        sizeof(wc),
        0);
    LELOG("dec ret[%d] [%d/%d]", ret, ENC_SIZE(AES_LEN, sizeof(wc) + 1), decLen);
    if (0 > ret) {
        return -4;
    }
    memcpy(&wc, buf, sizeof(wc));
}

    if(wc.magic != WIFICONFIG_MAGIC) {
        LELOGE("magic = %d", wc.magic);
        return -5;
    }
    sum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);
    if(wc.checksum != sum) {
        LELOGE("checksum = %d", wc.magic);
        return -6;
    }
    LELOG("Get ssid[%s] passwd[%s]", wc.ssid, wc.wap2passwd);
    {
        PrivateCfg cfg;
        lelinkStorageReadPrivateCfg(&cfg);
        LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                cfg.data.nwCfg.config.ssid,
                cfg.data.nwCfg.config.psk, 
                cfg.data.nwCfg.configStatus);
        strcpy((char *)cfg.data.nwCfg.config.ssid, (const char *)wc.ssid);
        cfg.data.nwCfg.config.ssid_len = strlen((const char *)wc.ssid);
        cfg.data.nwCfg.config.ssid[cfg.data.nwCfg.config.ssid_len] = '\0';
        strcpy((char *)cfg.data.nwCfg.config.psk, (const char *)wc.wap2passwd);
        cfg.data.nwCfg.config.psk_len = strlen((const char *)wc.wap2passwd);
        cfg.data.nwCfg.config.psk[cfg.data.nwCfg.config.psk_len] = '\0';
        cfg.data.nwCfg.configStatus = 1;
        ret = lelinkStorageWritePrivateCfg(&cfg);
        LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
    }
    return ret;
}

int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout, const char *aesKey)
{
    void *ctx;
    uint16_t port = 4911;
    int i, ret, count, delay = 1000; // ms
    char ipaddr[32] = "192.168.10.1";
    wificonfig_t wc = { WIFICONFIG_MAGIC, WIFICONFIG_SOFTAP_VER, 0 };
    uint8_t iv[AES_LEN] = { 0 };
    uint8_t key[AES_LEN] = { 0 };
    uint8_t beEncData[ENC_SIZE(AES_LEN, sizeof(wc) + 1)] = {0};
    uint32_t encLen = sizeof(wc);

    if (NULL == aesKey) {
        aesKey = "157e835e6c0bc55474abcd91e00e6979";
    }

    ctx  = lelinkNwNew(NULL, 0, 0, NULL);
    if(!ctx) {
        LELOGE("New link error");
        return -2;
    }
    count = timeout / delay + 1;
    strncpy((char *)wc.ssid, ssid, sizeof(wc.ssid));
    strncpy((char *)wc.wap2passwd, passwd, sizeof(wc.wap2passwd));
    wc.checksum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);


    memcpy(beEncData, &wc, sizeof(wc));
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    LELOG("AES %s", aesKey);
    hexStr2bytes(aesKey, key, AES_LEN);
    ret = aes(iv, 
        key, 
        (uint8_t *)&beEncData,
        &encLen, /* in-len/out-enc size */
        sizeof(wc),
        1);
    LELOG("enc ret[%d] [%d/%d] sizeof(wc)[%d]", ret, ENC_SIZE(AES_LEN, sizeof(wc) + 1), encLen, sizeof(wc));
    if (0 > ret) {
        lelinkNwDelete(ctx);
        return -3;
    }
#if 0
        ret = aes(iv, 
            key, 
            (uint8_t *)&beEncData,
            &encLen, /* in-len/out-enc size */
            sizeof(wc),
            0);
        LELOG("dec ret[%d]", ret);
        memcpy(&wc, beEncData, sizeof(wc));
#endif

    for( i = 0; i < count; i++ ) {
        LELOG("Send wifi configure, [%s:%s][%d]...", ssid, passwd, delay);
        halDelayms(delay);
        ret = nwUDPSendto(ctx, ipaddr, port, (uint8_t *)beEncData, encLen);
        if(ret <= 0 ) {
            LELOGE("nwUDPSendto ret = %d", ret);
        }
    }
    if(ctx) {
        lelinkNwDelete(ctx);
    }
    return 0;
}

