#ifndef AIRCONFIG_H
#define AIRCONFIG_H

#include "leconfig.h"

#define MAX_CHANNEL_CARE 4

typedef enum {
    AIRCONFIG_NW_STATE_NONE,
    AIRCONFIG_NW_STATE_ERROR_PARAM,
    AIRCONFIG_NW_STATE_CHANNEL_LOCKED,
    AIRCONFIG_NW_STATE_COMPLETED
}airconfig_nw_state_t;

/*
 * to fill the item by sniffer info
 * data: the length or current package
 */
typedef struct {
    uint16_t data;
    uint8_t mac_src[6];
    uint8_t mac_dst[6];
    uint8_t mac_bssid[6];
}target_item_t;

/*
 * the max ssid or passwd is 32 byes
 */
typedef struct {
	char ssid[36];
	char psk[36];
}ap_passport_t;

/*
 * 功能: 收集monitor模式下收到的802.11的DATA数据信息
 *
 * 参数:
 *      item: target_item_t数据，即收到的一帧802.11数据的信息，详细参考数据结构target_item_t
 *      channel: 指示item的信息是来源于哪个802.11信道
 *      channel_locked: 配置过程中，用于收集信道的。当锁定完成后，该数组的结果中表示锁定了哪几个信道，最多MAX_CHANNEL_CARE个
 *      base: 用于返回基准值，用于下一步函数 airconfig_get_info()
 *
 * 返回值:
 *      1 - 表示锁定成功，下一步使用 airconfig_get_info() 进一步收集数据信息
 *
 * 注: 
 *      1, 调用该函数前，确保wifi数据信息是在monitor模式下收集到的
 *      2, 要求wifi收集数据的信道要在1-14之前，以约30ms的间隔切换
 *       
 */
int airconfig_do_sync(const target_item_t *item, int channel, int channel_locked[MAX_CHANNEL_CARE], uint16_t *base);
/*
 * 功能: 在成功使用 airconfig_do_sync() 锁定信道的前提下，进一步收集802.11数据信息
 *
 * 参数: 
 *      len: 收到802.11信息的DATA数据长度
 *      base: 由airconfig_do_sync()得到的基准值
 *      account: 在成功配置时，返回AP的信息，详细参考数据结构ap_passport_t 
 *      ssid: 如果通过知道该802.11的DATA数据来源的SSID, 则通过该参数传入，否则传入NULL
 *      len_ssid: ssid的字节长度
 *
 * 返回值:
 *      1 - 表示成功得到AP的信息，并将信息保存在account中
 *      2 - 注意处理超时，如果在该数据多次调用约2分钟后仍未成功，则应该回退到 airconfig_do_sync() 再次处理
 *
 * 注: 在调用该函数前，要求wifi信道的切换在锁定的几个间切换
 *
 */
int airconfig_get_info(int len, int base, ap_passport_t *account, const char *ssid, int len_ssid);

/*
 * 功能: 重置配置
 *
 * 返回值:
 *      0 - 重置成功，目前不会有失败的情况
 *
 * 注: 在结束一次配置后使用
 *
 */
int airconfig_reset(void);

#endif /* AIRCONFIG_H */
