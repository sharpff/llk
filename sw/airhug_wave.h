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

#ifdef __cplusplus
extern "C" {
#endif

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
int airhug_wave(const char *ssid, const char *passwd, void (*delayms)(uint16_t ms));

#ifdef __cplusplus
}
#endif

#endif    // #ifndef _AIRHUG_WAVE_H_

