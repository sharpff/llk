#include "utility.h"

uint8_t crc8(const uint8_t *ptr, uint16_t len) {
    uint8_t crc;
    uint8_t i;
    crc = 0;
    while (len--) {
        crc ^= *ptr++;
        for (i = 0; i < 8; i++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}

int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen) {
    // const char tab[] = "0123456789abcdef";
	const char tab[] = "0123456789ABCDEF";
	int i = 0;

	memset(dst, 0, dstLen);
    
	if (dstLen < srcLen * 2)
		srcLen = (dstLen - 1) / 2;

	for (i = 0; i < srcLen; i++) {
		*dst++ = tab[*src >> 4];
		*dst++ = tab[*src & 0x0f];
		src++;
	}

	return 0;
        
}


/* hex string to bytes*/
int hexStr2bytes(const char *hexStr, uint8_t *buf, int bufLen) {
	int i;
	int len;
    
	if (NULL == hexStr) {
		len = 0;
	}
	else {
		len = (int)strlen(hexStr) / 2;
        
		if (bufLen < len)
			len = bufLen;
	}
    
	// sscanf过于耗费内存，采用如下方式转换
    
	memset(buf, 0, bufLen);
    
	for (i = 0; i < len; i++) {
		char ch1, ch2;
		int val;
        
		ch1 = hexStr[i * 2];
		ch2 = hexStr[i * 2 + 1];
		if (ch1 >= '0' && ch1 <= '9')
			val = (ch1 - '0') * 16;
		else if (ch1 >= 'a' && ch1 <= 'f')
			val = ((ch1 - 'a') + 10) * 16;
		else if (ch1 >= 'A' && ch1 <= 'F')
			val = ((ch1 - 'A') + 10) * 16;
		else
			return -1;
        
		if (ch2 >= '0' && ch2 <= '9')
			val += ch2 - '0';
		else if (ch2 >= 'a' && ch2 <= 'f')
			val += (ch2 - 'a') + 10;
		else if (ch2 >= 'A' && ch2 <= 'F')
			val += (ch2 - 'A') + 10;
		else
			return -1;
        
		buf[i] = val & 0xff;
	}
    
	return 0;
}

uint16_t genRand() {
    return 123;
}

uint16_t genSeqId() {
    static uint8_t seq;
    if (0 == seq) {
        seq++;
    }
    return seq++;
}