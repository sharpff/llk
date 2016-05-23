#include "leconfig.h"
#include "ota.h"

/*
 * 功能: 根据url打开http的session
 *
 * 参数: 
 *      info: session信息
 *      url: http/https地址
 *
 * 返回值:
 *      0 - 成功打开. 其它表示失败
 */
int halHttpOpen(OTAInfo_t *info, const char *url) {
    return 0;
}

/*
 * 功能: 关闭http的session
 *
 * 参数: 
 *      info: session信息
 */
void halHttpClose(OTAInfo_t *info) {
}

/*
 * 功能: 升级固件
 *
 * 参数: 
 *      info: session信息
 *
 * 返回值:
 *      0 - 成功升级. 其它表示失败
 */
int halUpdateFirmware(OTAInfo_t *info) {
    return 0;
}

/*
 * 功能: 升级其它(固件脚本,联动脚本等等)
 *
 * 参数: 
 *      info: session信息
 *
 * 返回值:
 *      0 - 成功升级. 其它表示失败
 */
uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen) {
    return 0;
}

