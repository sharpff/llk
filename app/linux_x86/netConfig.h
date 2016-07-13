/*
 * netConfig.h
 *
 * Create on: 2016-07-05
 *
 *    Author: feiguoyou@le.com
 *
 */

/*
 * 使用说明
 *
 * 1、本源码示例说明linux平台下wifi的配置接口实现，仅供参考。
 * 2、源码依赖于pcap库，所以要求系统中安装该库。
 * 3、代码中使用命令iwconfig切换信道，所以要求系统中包含该命令。
 * 4、代码中使用网口: wlan0
 * 5、本程序中没有涉及wifi模式切换的部分，因此要手动改变网卡的模式，参考命令如下:
 *      a, ifconfig wlan0 down
 *      b, iwconfig wlan0 mode monitor/managed
 *      c, ifconfig wlan0 up
 * 6、本代码只针对Realtek的网卡处理，其它网卡可能需要相应适配。
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

