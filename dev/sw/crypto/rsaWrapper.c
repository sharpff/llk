#include "rsaWrapper.h"
#include "pack.h"

int rsaEncrypt(const uint8_t *pubkey, int pubkeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {
	int ret, i = 0;
	int pieces = ENC_PIECES(RSA_RAW_LEN, inputLen);
	// LELOG("pieces[%d]\r\n", pieces);
	int encLen = 0, blockSize = RSA_RAW_LEN;


	for (i = 0; i < pieces; i++) {
		if (i + 1 == pieces) {
			blockSize = (inputLen % RSA_RAW_LEN) ? (inputLen % RSA_RAW_LEN) : RSA_RAW_LEN;
			// LELOG("i[%d], [%d][%s]\r\n", i, 	blockSize, input + i*blockSize);
		}
		halRsaInit();
		ret = halRsaEncrypt(pubkey, pubkeyLen, input + i*RSA_RAW_LEN, blockSize, out + encLen, outLen - encLen);
		halRsaExit();
		if (0 >= ret) {
			return ret;
		}
		encLen += RSA_LEN;
	}

	return encLen;
}

int rsaDecrypt(const uint8_t *prikey, int prikeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {
	int ret, i = 0;
	int pieces = ENC_PIECES(RSA_LEN, inputLen);
	// LELOG("pieces[%d]\r\n", pieces);
	int decLen = 0, blockSize = RSA_LEN;

	for (i = 0; i < pieces; i++) {
		halRsaInit();
		ret = halRsaDecrypt(prikey, prikeyLen, input + i*blockSize, blockSize, out + decLen, outLen - decLen);
		halRsaExit();
		if (0 >= ret) {
			return ret;
		}
		decLen += ret;
		// LELOG("decLen[%d], ret[%d]\r\n", decLen, ret);
	}
	return decLen;
}

int rsaVerify(const uint8_t* pubkey, int pubkeyLen, const uint8_t *raw, int rawLen, const uint8_t *sig, int sigLen)
{
	int ret = 0;
	halRsaInit();
	ret = halRsaVerify(pubkey, pubkeyLen, raw, rawLen, sig, sigLen);
	halRsaExit();
	return ret;
}

