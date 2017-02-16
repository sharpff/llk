/*
 * airhug_wave.h
 *
 * Create on: 2017-01-20
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#ifndef _AIRHUG_WAVE_H_
#define _AIRHUG_WAVE_H_

#include <stdint.h>

#define AIRHUG_VERSION      (0x01)
#define AIRHUG_MAX_LEN      (255)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 功能: 发送数据
 *
 * 参数: 
 *      data - data buf 
 *      len - data len
 *      delayms - 延时毫秒回调
 *
 * 返回值:
 *      0 - 成功发送一次数据
 *     -1 - 发送出错
 *
 */
int airhug_wave(const uint8_t *data, uint16_t len, void (*delayms)(uint16_t ms));

/*
 * 功能: 发送配置数据
 *
 * 参数: 
 *      ssid   - ssid 字符串
 *      passwd - passwd 字符串
 *      delayms - 延时毫秒回调
 *
 * 返回值:
 *      0 - 成功发送一次数据
 *     -1 - 发送出错
 *
 */
int airhug_wave_ext(const char *ssid, const char *passwd, void (*delayms)(uint16_t ms));

#ifdef __cplusplus
}
#endif

#endif    // #ifndef _AIRHUG_WAVE_H_

