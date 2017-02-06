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

int airhug_wave(char *ssid, char *passwd, void (*delayms)(uint16_t ms));

#ifdef __cplusplus
}
#endif

#endif    // #ifndef _AIRHUG_WAVE_H_

