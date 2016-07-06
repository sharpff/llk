/*
 * netConfig.h
 *
 * Create on: 2016-07-05
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#ifndef _NETCONFIG_H_
#define _NETCONFIG_H_

#define CHANNEL_LOCK_TIMEOUT    (20 * 1)

/*
 * 功能: 启动monitor模式
 *
 * 参数:
 *      无
 *
 * 返回值:
 *      0 - 成功启动
 *
 */
int netConfigStart(void);


/*
 * 功能: 检查在monitor模式下是否已经得到AP配置
 *
 * 参数:
 *      无
 *
 * 返回值:
 *      0 - 已经得到
 *
 */
int netConfigCheck(void);

#endif    // #ifndef _NETCONFIG_H_

