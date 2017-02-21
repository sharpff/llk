/*
 * rc4Wrapper.h
 *
 * Create on: 2017-02-13
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#ifndef _RC4WRAPPER_H_
#define _RC4WRAPPER_H_

#include "leconfig.h"

void rc4_init(uint8_t s[256], uint8_t *key, uint32_t len);

void rc4_crypt(uint8_t s[256], uint8_t *data, uint32_t len);

#endif    // #ifndef _RC4WRAPPER_H_

