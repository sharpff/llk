#include <stdio.h>
#include <assert.h>
#include "halHeader.h"
#include "httpc.h"
#include "leconfig.h"
#include "ota.h"
#include "MICODefine.h"
#include "CheckSumUtils.h"

#define FOTA_BUF_SIZE    (1024*4)
#define TMP_BUF_LEN 1024

static mico_Context_t *context;
static size_t httpFetchData(void *priv, void *buf, size_t max_len);

void mico_write_ota_set (int len, uint16_t crc)
{
    mico_logic_partition_t* ota_partition = MicoFlashGetInfo (MICO_PARTITION_OTA_TEMP);
    APPLOG("mico_write_ota_set [%x] [%d]",ota_partition->partition_start_addr, ota_partition->partition_length);

    memset (&context->flashContentInRam.bootTable, 0, sizeof (boot_table_t));
    context->flashContentInRam.bootTable.length = len;
    context->flashContentInRam.bootTable.start_address = ota_partition->partition_start_addr;
    context->flashContentInRam.bootTable.type = 'A';
    context->flashContentInRam.bootTable.upgrade_type = 'U';
    context->flashContentInRam.bootTable.crc = crc;
    context->flashContentInRam.micoSystemConfig.seed |= MAGICNUM_FOTA;
    MICOUpdateConfiguration (context);
}

int halHttpOpen(OTAInfo_t *info, const char *url) {
    int status = -1;
    char tmpurl[MAX_BUF] = {0};
    http_resp_t *resp;
    http_session_t session;

    info->session = NULL;
    strcpy(tmpurl, url);
    url = tmpurl;
again:
    status = httpc_get(url, &session, &resp, NULL);
    if (status != 0) {
        APPLOGE("Unable to connect to server");
        goto err_out;
    }
    if (resp->status_code > 300 && resp->status_code < 400) {
        char *pv = NULL;
        status = http_get_response_hdr_value(session, "Location", &pv);
        if(status) {
            APPLOGE("Can't get moved Location, status = %d", status);
            goto err_out;
        }
        APPLOGW("Http moved:%s", pv);
        strcpy(tmpurl, pv);
        http_close_session(&session);
        goto again;
    } else if (resp->status_code != 200) {
        APPLOGE("HTTP Error %d", resp->status_code);
        goto err_out;
    }
    if (resp->chunked) {
        APPLOGE("HTTP chunked fs update is not supported");
        goto err_out;
    }
    if (resp->content_length <= 0) {
        APPLOGE("HTTP size(%d) error", resp->content_length);
        goto err_out;
    }

    info->session = (void *)session;
    info->imgLen = resp->content_length;
    APPLOGE("HTTP content_length [%d]", resp->content_length);
    context = (mico_Context_t *)halMalloc (sizeof (mico_Context_t));
    memset (context, 0x0, sizeof (mico_Context_t));
    MICOReadConfiguration (context);
    return 0;
err_out:
    return -1;
}

void halHttpClose(OTAInfo_t *info) {
    if(info && info->session) {
        http_close_session((http_session_t *)&info->session);
        info->session = NULL;
        halFree(context);
    }
}

int halUpdateFirmware(OTAInfo_t *info) {
    int32_t ret = 0;
    OSStatus err;
    volatile uint32_t out_count;
    uint8_t md5_recv[16];
    uint8_t md5_calc[16];
    md5_context ctx;
    uint16_t crc = 0;
    CRC16_Context contex;
    uint32_t flashaddr;
    int len, left, totalSize = 0, i = 0;
    mico_logic_partition_t* ota_partition = MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP );
    char* buf = halMalloc(FOTA_BUF_SIZE);
    
    if (buf == NULL) {
        APPLOGE("buf malloc failed.");
        return -3;
    }

    do {
        int nSize = 0;
        nSize = httpFetchData(info, buf, FOTA_BUF_SIZE);
        if (0 >= nSize) {
                APPLOG("httpFetchData OVER [%d]", nSize);
                break;
        }
        // APPLOG("has read bytes [%d]", nSize);
        err = MicoFlashWrite(MICO_PARTITION_OTA_TEMP, &out_count, (const uint8_t*)buf, nSize);
        //APPLOGE("MicoFlashWrite write_ret[%d] [%d] [%d] [%d]", err, data_len, count, out_count);
        require_noerr(err, flashErrExit);
        totalSize += nSize;
    } while (totalSize < info->imgLen);
    	
    if(totalSize != info->imgLen) {
        APPLOGE("halUpdateFirmware data wrong, need %d bytes, but now %d bytes", info->imgLen, totalSize);
        halFree(buf);
        return -5;
    }
        
    APPLOG("Download result success");
    
    ret = lelinkVerify(ota_partition->partition_start_addr, totalSize);

    if ( 0 == ret ) {
        APPLOG("OTA SUCCESS. Rebooting [%d]...\r\n", totalSize);
        memset(buf, 0, FOTA_BUF_SIZE);
        flashaddr = totalSize;
        InitMd5( &ctx );
        CRC16_Init( &contex );
        flashaddr = 0;
        left = totalSize;
        while(left > 0) {
            if (left > TMP_BUF_LEN) {
                len = TMP_BUF_LEN;
            } else {
                len = left;
            }
            left -= len;
            MicoFlashRead(MICO_PARTITION_OTA_TEMP, &flashaddr, (uint8_t *)buf, len);
            Md5Update( &ctx, (uint8_t *)buf, len);
            CRC16_Update( &contex, buf, len );
        }
        Md5Final( &ctx, md5_calc );
        CRC16_Final( &contex, &crc );
        flashaddr = totalSize;
        APPLOG("crc [%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x]\r\n", 
               md5_calc[0],md5_calc[1],md5_calc[2],md5_calc[3],md5_calc[4],md5_calc[5],
               md5_calc[6],md5_calc[7],md5_calc[8],md5_calc[9],md5_calc[10],md5_calc[11],
               md5_calc[12],md5_calc[13],md5_calc[14],md5_calc[15]);
        MicoFlashWrite(MICO_PARTITION_OTA_TEMP, &flashaddr, (uint8_t *)md5_calc, 16);
        mico_write_ota_set(totalSize, crc);
        APPLOG("OTA SUCCESS. Rebooting len[%d] crc[%d]\r\n", totalSize, crc);
        MicoSystemReboot();
    } else {
          halFree(buf);
          return -1;
    }
    halFree(buf);
    return 0;
flashErrExit:
    APPLOGE("fail to write flash, ret = %d", ret);
    halFree(buf);
    return -4;
}

uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen) {
    int ret = 0, aSize = 256, nSize = 0;

    if(info == NULL || buf == NULL) {
        APPLOGE("Update script paremeter error!");
        return -1;
    }
    if(info->imgLen > MAX_PROFILE_SIZE) {
        APPLOGE("Script too large!");
        return -2;
    }
    APPLOG("Clear script...");
    
    while((ret = httpFetchData(info, &buf[nSize], aSize)) > 0) {
        if (nSize + ret > bufLen) {
            break;
        }
        nSize += ret;
    }
    if(nSize != info->imgLen) {
        APPLOGE("Get script data wrong, need %d bytes, but now %d bytes", info->imgLen, nSize);
        return -3;
    }
    APPLOG("Update script successed [%d]", nSize);
    return nSize;
}

static size_t httpFetchData(void *priv, void *buf, size_t max_len)
{
    int ret = 0;
    OTAInfo_t *info = (OTAInfo_t *) priv;

    if((ret = http_read_content((http_session_t)info->session, buf, max_len)) > 0){
        info->nowLen += ret;
    }
    //APPLOG("%d/%d", info->nowLen, info->imgLen);
    return ret;
}

void ota_fw(void)
{
    int type = 2;
    char url[64] = "http://115.182.63.167/zhiwei/mico_app.bin";
    leOTA(type, url, NULL, 0);
}

void ota_script(void)
{
    int type = 4;
    char url[64] = "http://115.182.63.167/zhiwei/emw3081.lua";
    leOTA(type, url, NULL, 0);
}

