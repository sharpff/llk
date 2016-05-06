#ifndef __MD5N_H__
#define __MD5N_H__

#include "leconfig.h"

void md5(uint8_t *input, uint32_t inputLen, uint8_t output[MD5_LEN]);

#endif