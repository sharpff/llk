#include "halHeader.h"

/*
 * 功能: 配置wifi进入monitor模式
 *
 * 参数:
 *      ptr: 保留
 *      ptrLen: 保留
 *
 * 返回值:
 *      1 - 成功配置, 其它表示失败
 *
 */
int halDoConfig(void *ptr, int ptrLen) {
    /*
     * 在进入monitor模式后, 调用以下接口(airconfig.h)得到ssid及passwd
     * int airconfig_do_sync(const target_item_t *item, int channel, int channel_locked[MAX_CHANNEL_CARE], uint16_t *base);
     * int airconfig_get_info(int len, int base, ap_passport_t *account, const char *ssid, int len_ssid);
     * int airconfig_reset(void);
     */
    return 1;
}

/*
 * 功能: 停止wifi的monitor模式
 *
 * 返回值:
 *      1 - 成功停止, 其它表示失败
 *
 */
int halStopConfig(void) {
    return 1;
}

/*
 * 功能: 查看是否得到wifi的配置
 *
 * 参数:
 *      ptr: 保留
 *      ptrLen: 保留
 *
 * 返回值:
 *      1 - 得到配置, 其它表示失败
 */
int halDoConfiguring(void *ptr, int ptrLen) {
	return 1;
}

/*
 * 功能: 请求wifi连接AP
 *
 * 参数:
 *      ptr: 指向结构PrivateCfg 
 *
 * 返回值:
 *      1 - 正在连接AP
 */
int halDoApConnect(void *ptr, int ptrLen) {
    return 1;
}

/*
 * 功能: 查看是否成功连接上AP
 *
 * 参数:
 *      ptr: 保留
 *      ptrLen: 保留
 *
 * 返回值:
 *      1 - 连接AP成功, 其它表示失败
 */
int halDoApConnecting(void *ptr, int ptrLen) {
    return 1;
}


/*
 * 功能: 启动wifi的AP功能模式
 *
 * 参数: 
 *      ssid: AP的名称
 *      wpa2_passphrase: AP的wpa2密码
 *
 * 返回值:
 *      0 - 成功启动, 其它表示失败
 */
int halSoftApStart(const char *ssid, char *wpa2_passphrase, uint8_t *aesKey, int aesKeyLen) {
    return 0;
}

/*
 * 功能: 停止wifi的AP功能模式
 *
 * 返回值:
 *      0 - 成功启动, 其它表示失败
 */
int halSoftApStop(int success) {
    return 0;
}

