/*
 * rc4Wrapper.c
 *
 * Create on: 2017-02-13
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

// #include <stdio.h>
#include "rc4Wrapper.h"

/*初始化函数*/
void rc4_init(uint8_t s[256], uint8_t *key, uint32_t len)
{
	uint8_t tmp = 0;
	int i = 0, j = 0;
	char k[256] = {0};

	for (i = 0; i < 256; i++) {
		s[i] = i;
		k[i] = key[i % len];
	}
	for (i = 0; i < 256; i++) {
		tmp = s[i];
		j = (j + s[i] + k[i]) % 256;
		s[i] = s[j];
		s[j] = tmp;
	}
}

/*加解密*/
void rc4_crypt(uint8_t s[256], uint8_t *data, uint32_t len)
{
	uint8_t tmp;
	uint32_t k = 0;
	int i = 0, j = 0, t = 0;

	for (k = 0; k < len; k++) {
		tmp = s[i];
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		s[i] = s[j];
		s[j] = tmp;
		t = (s[i] + s[j]) % 256;
		data[k] ^= s[t];
	}
}

