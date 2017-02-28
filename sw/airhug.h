/*
 * airhug.h
 *
 * Create on: 2017-01-19
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#ifndef _AIRHUG_H_
#define _AIRHUG_H_

// #include <stdio.h>
// #include <stdint.h>
#include "leconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 功能: 开始一次新的配置，清空上次保存的记录数据
 *
 * 参数: 
 *      无
 *
 * 返回值:
 *      无
 *
 */
void airhug_reset(void);

/*
 * 功能: 分析网络数据
 *
 * 参数: 
 *      src : 数据原MAC
 *      dst : 目标的MAC
 *      bssid : 路由MAC
 *
 * 返回值:
 *       0 : 继续
 *       1 : 锁定数据源
 *       2 : 成功得到配置信息
 *      -1 : 出错
 *
 */
int airhug_feed_data(const uint8_t *src, const uint8_t *dst, const uint8_t *bssid, uint32_t datalen);

/*
 * 功能: 得到配置信息
 *
 * 参数: 
 *         ssid : ssid数据缓冲区
 *      ssidlen : ssid 空间大小
 *       passwd : passwd数据缓冲区
 *    passwdlen : passwd 空间大小
 *
 * 返回值:
 *       0 : 成功得到数据(字符串)
 *      -1 : 出错
 *
 */
int airhug_get(char *ssid, uint16_t ssidlen, char *passwd, uint16_t passwdlen);

#ifdef __cplusplus
}
#endif

#endif    // #ifndef _AIRHUG_H_

