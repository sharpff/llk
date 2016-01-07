#ifdef __LE_SDK__
#undef __LE_SDK__
#endif
#include "leconfig.h"
// #include "halHeader.h"


//#include <stdio.h>
//#include <string.h>


#define LELINK_ENC3_S11 7
#define LELINK_ENC3_S12 12
#define LELINK_ENC3_S13 17
#define LELINK_ENC3_S14 22
#define LELINK_ENC3_S21 5
#define LELINK_ENC3_S22 9
#define LELINK_ENC3_S23 14
#define LELINK_ENC3_S24 20
#define LELINK_ENC3_S31 4
#define LELINK_ENC3_S32 11
#define LELINK_ENC3_S33 16
#define LELINK_ENC3_S34 23
#define LELINK_ENC3_S41 6
#define LELINK_ENC3_S42 10
#define LELINK_ENC3_S43 15
#define LELINK_ENC3_S44 21

/* MD5 context. */
typedef struct {
	uint32_t state[4];                                   /* state (ABCD) */
	uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
	uint8_t buffer[64];                         /* input buffer */
} lelink_enc3_ctx_t;

static void lelink_enc3_transform(uint32_t a[4], uint8_t b[64]);
static void lelink_enc3_encode(uint8_t *, uint32_t *, uint32_t);
static void lelink_enc3_decode(uint32_t *, uint8_t *, uint32_t);

static uint8_t lelink_enc3_padding[64] =
{
	0x80,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

#define LELINK_ENC3_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define LELINK_ENC3_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define LELINK_ENC3_H(x, y, z) ((x) ^ (y) ^ (z))
#define LELINK_ENC3_I(x, y, z) ((y) ^ ((x) | (~z)))


#define LELINK_ENC3_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define LELINK_ENC3_FF(a, b, c, d, x, s, ac) { \
(a) += LELINK_ENC3_F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
(a) = LELINK_ENC3_ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define LELINK_ENC3_GG(a, b, c, d, x, s, ac) { \
(a) += LELINK_ENC3_G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
(a) = LELINK_ENC3_ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define LELINK_ENC3_HH(a, b, c, d, x, s, ac) { \
(a) += LELINK_ENC3_H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
(a) = LELINK_ENC3_ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define LELINK_ENC3_II(a, b, c, d, x, s, ac) { \
(a) += LELINK_ENC3_I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
(a) = LELINK_ENC3_ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}


static void lelink_enc3_init(lelink_enc3_ctx_t *context) {
	context->count[0] = context->count[1] = 0;
    
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}


static void lelink_enc3_update(lelink_enc3_ctx_t *context, uint8_t *input, uint32_t inputLen) {
	uint32_t i, index, partLen;
    
	index = (uint32_t)((context->count[0] >> 3) & 0x3F);
    
	if ((context->count[0] += ((uint32_t)inputLen << 3))
	    < ((uint32_t)inputLen << 3))
		context->count[1]++;
	context->count[1] += ((uint32_t)inputLen >> 29);
    
	partLen = 64 - index;
    
    
	if (inputLen >= partLen) {
		memcpy((uint8_t *)&context->buffer[index], (uint8_t *)input, partLen);
		lelink_enc3_transform(context->state, context->buffer);
        
		for (i = partLen; i + 63 < inputLen; i += 64)
			lelink_enc3_transform(context->state, &input[i]);
        
		index = 0;
	}
	else
		i = 0;
    
	memcpy((uint8_t *)&context->buffer[index],
		(uint8_t *)&input[i],
		inputLen-i);
}

static void lelink_enc3_final(uint8_t digest[16], lelink_enc3_ctx_t * context) {
	uint8_t bits[8];
	uint32_t index, padLen;
    
	lelink_enc3_encode(bits, context->count, 8);
    
	index = (uint32_t)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	lelink_enc3_update(context, lelink_enc3_padding, padLen);
    
	lelink_enc3_update(context, bits, 8);
    
	lelink_enc3_encode(digest, context->state, 16);
    
	memset((uint8_t *)context, 0, sizeof(*context));
}

static void lelink_enc3_transform(uint32_t state[4], uint8_t block[64]) {
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    
	lelink_enc3_decode(x, block, 64);
    
	/* Round 1 */
	LELINK_ENC3_FF(a, b, c, d, x[0], LELINK_ENC3_S11, 0xd76aa478); /* 1 */
	LELINK_ENC3_FF(d, a, b, c, x[1], LELINK_ENC3_S12, 0xe8c7b756); /* 2 */
	LELINK_ENC3_FF(c, d, a, b, x[2], LELINK_ENC3_S13, 0x242070db); /* 3 */
	LELINK_ENC3_FF(b, c, d, a, x[3], LELINK_ENC3_S14, 0xc1bdceee); /* 4 */
	LELINK_ENC3_FF(a, b, c, d, x[4], LELINK_ENC3_S11, 0xf57c0faf); /* 5 */
	LELINK_ENC3_FF(d, a, b, c, x[5], LELINK_ENC3_S12, 0x4787c62a); /* 6 */
	LELINK_ENC3_FF(c, d, a, b, x[6], LELINK_ENC3_S13, 0xa8304613); /* 7 */
	LELINK_ENC3_FF(b, c, d, a, x[7], LELINK_ENC3_S14, 0xfd469501); /* 8 */
	LELINK_ENC3_FF(a, b, c, d, x[8], LELINK_ENC3_S11, 0x698098d8); /* 9 */
	LELINK_ENC3_FF(d, a, b, c, x[9], LELINK_ENC3_S12, 0x8b44f7af); /* 10 */
	LELINK_ENC3_FF(c, d, a, b, x[10], LELINK_ENC3_S13, 0xffff5bb1); /* 11 */
	LELINK_ENC3_FF(b, c, d, a, x[11], LELINK_ENC3_S14, 0x895cd7be); /* 12 */
	LELINK_ENC3_FF(a, b, c, d, x[12], LELINK_ENC3_S11, 0x6b901122); /* 13 */
	LELINK_ENC3_FF(d, a, b, c, x[13], LELINK_ENC3_S12, 0xfd987193); /* 14 */
	LELINK_ENC3_FF(c, d, a, b, x[14], LELINK_ENC3_S13, 0xa679438e); /* 15 */
	LELINK_ENC3_FF(b, c, d, a, x[15], LELINK_ENC3_S14, 0x49b40821); /* 16 */
    
	/* Round 2 */
	LELINK_ENC3_GG(a, b, c, d, x[1], LELINK_ENC3_S21, 0xf61e2562); /* 17 */
	LELINK_ENC3_GG(d, a, b, c, x[6], LELINK_ENC3_S22, 0xc040b340); /* 18 */
	LELINK_ENC3_GG(c, d, a, b, x[11], LELINK_ENC3_S23, 0x265e5a51); /* 19 */
	LELINK_ENC3_GG(b, c, d, a, x[0], LELINK_ENC3_S24, 0xe9b6c7aa); /* 20 */
	LELINK_ENC3_GG(a, b, c, d, x[5], LELINK_ENC3_S21, 0xd62f105d); /* 21 */
	LELINK_ENC3_GG(d, a, b, c, x[10], LELINK_ENC3_S22, 0x2441453); /* 22 */
	LELINK_ENC3_GG(c, d, a, b, x[15], LELINK_ENC3_S23, 0xd8a1e681); /* 23 */
	LELINK_ENC3_GG(b, c, d, a, x[4], LELINK_ENC3_S24, 0xe7d3fbc8); /* 24 */
	LELINK_ENC3_GG(a, b, c, d, x[9], LELINK_ENC3_S21, 0x21e1cde6); /* 25 */
	LELINK_ENC3_GG(d, a, b, c, x[14], LELINK_ENC3_S22, 0xc33707d6); /* 26 */
	LELINK_ENC3_GG(c, d, a, b, x[3], LELINK_ENC3_S23, 0xf4d50d87); /* 27 */
	LELINK_ENC3_GG(b, c, d, a, x[8], LELINK_ENC3_S24, 0x455a14ed); /* 28 */
	LELINK_ENC3_GG(a, b, c, d, x[13], LELINK_ENC3_S21, 0xa9e3e905); /* 29 */
	LELINK_ENC3_GG(d, a, b, c, x[2], LELINK_ENC3_S22, 0xfcefa3f8); /* 30 */
	LELINK_ENC3_GG(c, d, a, b, x[7], LELINK_ENC3_S23, 0x676f02d9); /* 31 */
	LELINK_ENC3_GG(b, c, d, a, x[12], LELINK_ENC3_S24, 0x8d2a4c8a); /* 32 */
    
	/* Round 3 */
	LELINK_ENC3_HH(a, b, c, d, x[5], LELINK_ENC3_S31, 0xfffa3942); /* 33 */
	LELINK_ENC3_HH(d, a, b, c, x[8], LELINK_ENC3_S32, 0x8771f681); /* 34 */
	LELINK_ENC3_HH(c, d, a, b, x[11], LELINK_ENC3_S33, 0x6d9d6122); /* 35 */
	LELINK_ENC3_HH(b, c, d, a, x[14], LELINK_ENC3_S34, 0xfde5380c); /* 36 */
	LELINK_ENC3_HH(a, b, c, d, x[1], LELINK_ENC3_S31, 0xa4beea44); /* 37 */
	LELINK_ENC3_HH(d, a, b, c, x[4], LELINK_ENC3_S32, 0x4bdecfa9); /* 38 */
	LELINK_ENC3_HH(c, d, a, b, x[7], LELINK_ENC3_S33, 0xf6bb4b60); /* 39 */
	LELINK_ENC3_HH(b, c, d, a, x[10], LELINK_ENC3_S34, 0xbebfbc70); /* 40 */
	LELINK_ENC3_HH(a, b, c, d, x[13], LELINK_ENC3_S31, 0x289b7ec6); /* 41 */
	LELINK_ENC3_HH(d, a, b, c, x[0], LELINK_ENC3_S32, 0xeaa127fa); /* 42 */
	LELINK_ENC3_HH(c, d, a, b, x[3], LELINK_ENC3_S33, 0xd4ef3085); /* 43 */
	LELINK_ENC3_HH(b, c, d, a, x[6], LELINK_ENC3_S34, 0x4881d05); /* 44 */
	LELINK_ENC3_HH(a, b, c, d, x[9], LELINK_ENC3_S31, 0xd9d4d039); /* 45 */
	LELINK_ENC3_HH(d, a, b, c, x[12], LELINK_ENC3_S32, 0xe6db99e5); /* 46 */
	LELINK_ENC3_HH(c, d, a, b, x[15], LELINK_ENC3_S33, 0x1fa27cf8); /* 47 */
	LELINK_ENC3_HH(b, c, d, a, x[2], LELINK_ENC3_S34, 0xc4ac5665); /* 48 */
    
	/* Round 4 */
	LELINK_ENC3_II(a, b, c, d, x[0], LELINK_ENC3_S41, 0xf4292244); /* 49 */
	LELINK_ENC3_II(d, a, b, c, x[7], LELINK_ENC3_S42, 0x432aff97); /* 50 */
	LELINK_ENC3_II(c, d, a, b, x[14], LELINK_ENC3_S43, 0xab9423a7); /* 51 */
	LELINK_ENC3_II(b, c, d, a, x[5], LELINK_ENC3_S44, 0xfc93a039); /* 52 */
	LELINK_ENC3_II(a, b, c, d, x[12], LELINK_ENC3_S41, 0x655b59c3); /* 53 */
	LELINK_ENC3_II(d, a, b, c, x[3], LELINK_ENC3_S42, 0x8f0ccc92); /* 54 */
	LELINK_ENC3_II(c, d, a, b, x[10], LELINK_ENC3_S43, 0xffeff47d); /* 55 */
	LELINK_ENC3_II(b, c, d, a, x[1], LELINK_ENC3_S44, 0x85845dd1); /* 56 */
	LELINK_ENC3_II(a, b, c, d, x[8], LELINK_ENC3_S41, 0x6fa87e4f); /* 57 */
	LELINK_ENC3_II(d, a, b, c, x[15], LELINK_ENC3_S42, 0xfe2ce6e0); /* 58 */
	LELINK_ENC3_II(c, d, a, b, x[6], LELINK_ENC3_S43, 0xa3014314); /* 59 */
	LELINK_ENC3_II(b, c, d, a, x[13], LELINK_ENC3_S44, 0x4e0811a1); /* 60 */
	LELINK_ENC3_II(a, b, c, d, x[4], LELINK_ENC3_S41, 0xf7537e82); /* 61 */
	LELINK_ENC3_II(d, a, b, c, x[11], LELINK_ENC3_S42, 0xbd3af235); /* 62 */
	LELINK_ENC3_II(c, d, a, b, x[2], LELINK_ENC3_S43, 0x2ad7d2bb); /* 63 */
	LELINK_ENC3_II(b, c, d, a, x[9], LELINK_ENC3_S44, 0xeb86d391); /* 64 */
    
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
    
	memset((uint8_t *)x, 0, sizeof(x));
}

static void lelink_enc3_encode(uint8_t *output, uint32_t *input, uint32_t len) {
	uint32_t i, j;
    
	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (uint8_t)(input[i] & 0xff);
		output[j + 1] = (uint8_t)((input[i] >> 8) & 0xff);
		output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
		output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
	}
}


static void lelink_enc3_decode(uint32_t *output, uint8_t *input, uint32_t len) {
	uint32_t i, j;
    
	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) |
		(((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
}



//MD5
void halMD5(uint8_t *input, uint32_t inputlen, uint8_t output[16]) {
	lelink_enc3_ctx_t ctx;

	memset(&ctx, 0, sizeof(ctx));
	lelink_enc3_init(&ctx);
	lelink_enc3_update(&ctx, input, inputlen);
	lelink_enc3_final(output, &ctx);
}
