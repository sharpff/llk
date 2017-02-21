/*
 * softap.h
 *
 * Create on: 2017-02-20
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#ifndef _SOFTAP_H_
#define _SOFTAP_H_

#include "leconfig.h"

/*
 * 功能: 启动softap
 *
 * 返回值: 
 *      0 表示启动AP成功
 *
 */
int softApStart(void);

/*
 * 功能: 停止softap
 *
 * 返回值: 
 *      0 表示停止AP成功
 *
 */
int softApStop(int success);

/*
 * 功能: 在softap模式下，接收AP的配置信息
 *
 * 返回值: 
 *      0 表示成功接收到AP的信息
 *
 */
int softApCheck(void);

/*
 * 功能: 向网关发送wifi信息
 *
 * 参数:
 *      ssid - wifi名称
 *      passwd - wifi密码
 *      timeout - 发送超时时间
 *      aesKey - AES加密
 * 
 * 
 * 返回值: 
 *      0 表示成功发送
 *
 */
int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout, const char *aesKey);

#endif    // #ifndef _SOFTAP_H_

