#include "md5Wrapper.h"


//MD5
void md5(uint8_t *input, uint32_t inputLen, uint8_t output[MD5_LEN]) {
	halMD5(input, inputLen, output);
}
