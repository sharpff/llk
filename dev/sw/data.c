#include "leconfig.h"
#include "data.h"
#include "pack.h"
#include "protocol.h"
#include "utility.h"

// static uint8_t dynamicKeyAES[AES_LEN] = {0};

// AES
const uint8_t *getPreSharedIV() {
    // letv aes128 cbc pkcs5 iv
    // 4d90c52bea5259b95b53d33c63a706e1
	const static uint8_t preSharedIV[] = { 0x4d, 0x90, 0xc5, 0x2b, 0xea, 0x52, 0x59, 0xb9, 0x5b, 0x53, 0xd3, 0x3c, 0x63, 0xa7, 0x06, 0xe1 }; 
    return preSharedIV;
}

const uint8_t *getPreSharedToken() {
    // md5 of lelink token
    // 157e835e6c0bc55474abcd91e00e6979
    const static uint8_t preKeyAES[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x79}; 
    return preKeyAES;
}

const uint8_t *getTerminalToken() {
    // TODO: GEN IT BY UTC
#ifdef TEST_SDK
    const static uint8_t terminalAES[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x80}; 
#endif
#ifdef TEST_DEV
    const static uint8_t terminalAES[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x81}; 
#endif
    return terminalAES;
}

void getTerminalTokenStr(char *token, int len) {
    bytes2hexStr(getTerminalToken(), AES_LEN, (uint8_t *)token, len);
}

// void getDynamicTokenBin(uint16_t *token, int len) {
//     memcpy(token, getTerminalToken(), len);
// }

// 
int getPreSharedPublicKey(uint8_t *pubkey, int pubkeyLen) {
    // TODO: get pubkey from flash
    // md5 of lelink public key
    // 6e585704b0b8e7c422caf11eb1e671f3
    if (pubkeyLen < ginPubkeyGlobalDerLen) {
        return 0;
    }
    memcpy(pubkey, ginPubkeyGlobalDer, ginPubkeyGlobalDerLen);
    return ginPubkeyGlobalDerLen;
}

int getTerminalPublicKey(uint8_t *pubkey, int pubkeyLen) {
    // TODO: get pubkey from flash
    // md5 of lelink public key
    // 6e585704b0b8e7c422caf11eb1e671f3
    if (pubkeyLen < ginPubkeyDevKindDerLen) {
        return 0;
    }
    memcpy(pubkey, ginPubkeyDevKindDer, ginPubkeyDevKindDerLen);
    return ginPubkeyDevKindDerLen;
}

int getTerminalSignature(uint8_t *signature, int len) {
#ifdef TEST_SDK
    const uint8_t sig[] = {
        0x0f, 0x29, 0xae, 0x7a, 0xbf, 0xcc, 0x13, 0x91, 0x32, 0x39, 0x14, 0x87, 0xbc, 0xd1, 0x2f, 0x6e, 
        0x67, 0x8a, 0xb5, 0x72, 0xa0, 0xe2, 0x63, 0xf3, 0xee, 0x90, 0x40, 0x80, 0xfd, 0x89, 0x4b, 0x1d, 
        0x5d, 0x20, 0xb7, 0x88, 0x38, 0xf9, 0x07, 0xc8, 0x59, 0x81, 0xe7, 0xae, 0x8e, 0x97, 0x6f, 0xaa, 
        0x49, 0xea, 0xe9, 0xcd, 0x5d, 0x1d, 0x42, 0x45, 0xcc, 0xf5, 0x6f, 0x88, 0x09, 0xe0, 0xf9, 0xb6, 
        0x36, 0xbc, 0x41, 0x75, 0x1e, 0x80, 0x93, 0x05, 0x6f, 0xdf, 0x51, 0xa7, 0xc2, 0xe8, 0xa0, 0x90, 
        0x34, 0x5c, 0x23, 0x92, 0xba, 0x00, 0x40, 0x79, 0x32, 0xcd, 0x97, 0x10, 0xcf, 0xc4, 0xed, 0x7f, 
        0xba, 0x4b, 0x66, 0x1b, 0xa8, 0xa9, 0x4d, 0x15, 0xc0, 0x5b, 0xde, 0x13, 0xb6, 0xab, 0x66, 0x2f, 
        0xca, 0x2b, 0x7e, 0x81, 0x55, 0x0b, 0x82, 0x09, 0x67, 0xba, 0x48, 0xe9, 0xde, 0xd8, 0x5a, 0xe6 
    };
#endif

#ifdef TEST_DEV
    const uint8_t sig[] = {
        0x17, 0xb9, 0xd9, 0x78, 0x06, 0x80, 0xa0, 0x49, 0x32, 0xea, 0xf2, 0x08, 0x4d, 0x7e, 0x4b, 0xb6, 
        0xcd, 0xc5, 0xcf, 0x68, 0x93, 0x84, 0x91, 0xfa, 0x6b, 0xb8, 0x90, 0xcc, 0xa1, 0x9e, 0x8d, 0x61, 
        0x53, 0x08, 0x4f, 0x67, 0x41, 0x39, 0x6c, 0x9a, 0x53, 0x3c, 0xaf, 0xa8, 0x10, 0x93, 0x67, 0x44, 
        0xb6, 0x60, 0x81, 0x05, 0xea, 0xd7, 0x2b, 0xd4, 0x8b, 0xd7, 0xb2, 0xee, 0x07, 0xa9, 0x65, 0x9c, 
        0x98, 0xc7, 0xe7, 0xd5, 0xde, 0x6e, 0xe5, 0x78, 0x9f, 0x10, 0x3e, 0xd7, 0x15, 0x7c, 0x5c, 0x16, 
        0x55, 0xe4, 0xa5, 0xf0, 0x2d, 0xe2, 0xa3, 0x4e, 0x0a, 0x6e, 0x4c, 0x8a, 0x0a, 0xa7, 0x2b, 0x51, 
        0x88, 0x96, 0x8c, 0x86, 0x4b, 0x8b, 0x73, 0x5c, 0xf1, 0xaa, 0xdf, 0xa3, 0xa3, 0xee, 0xf2, 0x98, 
        0x0a, 0x9d, 0xd9, 0xfa, 0x40, 0x29, 0x17, 0x68, 0xba, 0x2c, 0xb8, 0x02, 0x8a, 0xbf, 0xd6, 0x7d 
    };
#endif

    if (len < sizeof(sig)) {
        return 0;
    }
    // TODO: this is a signature, it should be planted in flash
    // or for sdk, it should be built-in align version
    memcpy(signature, sig, sizeof(sig));
    return sizeof(sig);
}

void getTerminalUUID(uint8_t uuid[MAX_UUID]) {
    // TODO: read uuid from flash
#ifdef TEST_SDK
    const char *strUUID = "20151229202624-ad09941a-2264-4ff";
#endif
#ifdef TEST_DEV
    const char *strUUID = "10000100011000510005123456abcdef";
#endif
    memcpy(uuid, strUUID, strlen(strUUID));
}


void getOriServerIP(char IP[], int len, uint16_t *port) {
    const char *ip = REMOTE_IP;
    *port = REMOTE_PORT;
    memcpy(IP, (void *)ip, strlen(ip) > len ? len : strlen(ip));
}

uint16_t getProtocolVersion() {
    return 0x0100;
}


int getTerminalStatus(char *status, int len) {
    // TODO: read from cache
    const char *tmp = "{\"status\":{\"pwr\":1,\"mark\":3,\"level\":5},\"cloud\":1}";
    memcpy(status, tmp, strlen(tmp));
    return strlen(tmp);
}

/* TODO: Debug only */
const uint8_t ginStrPubkeyPem[] = PUBLIC_KEY_PEM;
const int ginStrPubkeyPemLen = sizeof( ginStrPubkeyPem );
const uint8_t ginStrPrikeyPem[] = PRIATE_KEY_PEM;
const int ginStrPrikeyPemLen = sizeof( ginStrPrikeyPem );

/* built-in global rsa pubkey */
const uint8_t ginPubkeyGlobalDer[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x91, 
    0xFE, 0xAF, 0x0E, 0xF7, 0x84, 0x2C, 0x5F, 0xDC, 0xA5, 0x5F, 
    0xD2, 0xF5, 0x68, 0x6D, 0x6D, 0x71, 0xCC, 0xBA, 0xC0, 0xB3, 
    0x98, 0x60, 0xBA, 0x63, 0x06, 0xFF, 0x5E, 0x58, 0xC7, 0x75, 
    0x4F, 0x0B, 0x49, 0x06, 0x57, 0x1F, 0x66, 0x29, 0x9E, 0x57, 
    0xE7, 0xF4, 0xFF, 0xB5, 0xAD, 0x90, 0xE0, 0xE7, 0x0C, 0x6F, 
    0x5F, 0x76, 0x5F, 0x33, 0x72, 0x5C, 0xB7, 0x27, 0xFF, 0x22, 
    0xE4, 0x1E, 0x54, 0x17, 0xBA, 0xB3, 0x58, 0x0E, 0x50, 0xEF, 
    0x88, 0x3F, 0xE1, 0x37, 0x18, 0x37, 0xDE, 0xEF, 0x79, 0x88, 
    0xFE, 0xC3, 0xC4, 0x4D, 0x3A, 0x61, 0x3A, 0xC9, 0x9B, 0x75, 
    0x95, 0xB1, 0x25, 0x2A, 0xB7, 0x32, 0xD6, 0x29, 0xBB, 0x1F, 
    0xA4, 0x88, 0xA6, 0xE1, 0x21, 0xA7, 0x55, 0x37, 0x50, 0x85, 
    0x67, 0x0D, 0x02, 0x9C, 0x17, 0xF4, 0x4B, 0x12, 0xAC, 0xB0, 
    0xB2, 0x13, 0x88, 0xCF, 0x3E, 0x3E, 0x75, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
const int ginPubkeyGlobalDerLen = sizeof( ginPubkeyGlobalDer );

const unsigned char ginPubkeySDK1Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x82, 
    0x24, 0x79, 0x05, 0xEA, 0x23, 0x81, 0x5F, 0xC3, 0x9B, 0x0A, 
    0x24, 0xB5, 0xB6, 0x6C, 0x8C, 0xEA, 0xD8, 0x2B, 0x53, 0x64, 
    0x2A, 0x47, 0x40, 0x72, 0xC9, 0xF3, 0x17, 0x8E, 0xFA, 0x75, 
    0xC7, 0x69, 0xCA, 0x89, 0x23, 0x40, 0x05, 0x7B, 0x63, 0x2C, 
    0x08, 0x84, 0x50, 0xF1, 0xC1, 0x33, 0x2A, 0x53, 0x84, 0x15, 
    0x12, 0x00, 0x39, 0x70, 0xEB, 0x33, 0x3B, 0x5C, 0xE0, 0x0D, 
    0x1E, 0x66, 0x96, 0x4A, 0x80, 0xCA, 0x58, 0xE8, 0xC3, 0x64, 
    0xA7, 0x8A, 0xF3, 0x6B, 0x99, 0x0E, 0x1F, 0xF8, 0x0B, 0x69, 
    0x3C, 0xD3, 0xAC, 0xCB, 0x6F, 0xCC, 0x22, 0x4E, 0xB8, 0x7E, 
    0x7B, 0xEA, 0x9E, 0x29, 0xA0, 0x62, 0x36, 0x76, 0x3C, 0x3C, 
    0xC3, 0x28, 0x04, 0x26, 0x2B, 0x36, 0xF3, 0xBF, 0x82, 0x73, 
    0x3C, 0x69, 0x4E, 0x06, 0x23, 0x28, 0x50, 0x9C, 0x67, 0xD3, 
    0x92, 0x07, 0x29, 0x4A, 0x6D, 0x9A, 0xAB, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
const int ginPubkeySDK1DerLen = sizeof(ginPubkeySDK1Der);

const unsigned char ginPubkeyDEV1Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xC2, 
    0xC9, 0x03, 0x30, 0xBD, 0x06, 0xAE, 0x23, 0x5F, 0xCF, 0x6E, 
    0xD4, 0x36, 0x17, 0x05, 0x53, 0x67, 0x00, 0xDC, 0x3D, 0xC0, 
    0xD7, 0x23, 0x59, 0xC4, 0x50, 0x8B, 0xEA, 0xB3, 0x3F, 0xAD, 
    0x96, 0x64, 0x33, 0xC2, 0x90, 0x96, 0x19, 0x43, 0xB0, 0x1D, 
    0xB6, 0x45, 0xDC, 0x05, 0x70, 0xF1, 0xBD, 0x73, 0xBC, 0x5E, 
    0x67, 0x5B, 0xF5, 0x14, 0xD1, 0xA1, 0x7C, 0xCF, 0x62, 0x8A, 
    0x0A, 0xAB, 0x66, 0xEA, 0x00, 0x78, 0x8D, 0x26, 0xE5, 0x6C, 
    0x96, 0x43, 0x52, 0x5B, 0xCC, 0x84, 0x04, 0xA2, 0x1A, 0x70, 
    0x94, 0x1A, 0x0D, 0xA5, 0x1D, 0xFD, 0xCC, 0x0E, 0xD8, 0x8C, 
    0x14, 0xE7, 0x86, 0xCB, 0x65, 0xE6, 0xBC, 0x12, 0xC3, 0x2D, 
    0x95, 0x4F, 0xDE, 0xE5, 0x3C, 0x80, 0x32, 0xDF, 0x4D, 0xF0, 
    0xAA, 0x5B, 0xE2, 0x56, 0xCD, 0x3A, 0xA8, 0x10, 0x16, 0x6B, 
    0x10, 0xAE, 0x48, 0x3D, 0x06, 0xE2, 0xFB, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
const int ginPubkeyDEV1DerLen = sizeof(ginPubkeyDEV1Der);

const unsigned char ginPubkeyDEV2Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x91, 
    0xCE, 0xE0, 0x39, 0xED, 0x8C, 0x76, 0xBC, 0x7E, 0xF3, 0xD5, 
    0x1A, 0x7D, 0x33, 0x3D, 0x23, 0xBF, 0xCB, 0x4C, 0xE3, 0x26, 
    0x37, 0xAA, 0x73, 0xE0, 0x93, 0x4F, 0x8E, 0xD0, 0x40, 0x7D, 
    0x86, 0x19, 0x91, 0xDD, 0xFE, 0x46, 0xEB, 0x05, 0x01, 0x6A, 
    0x96, 0xDB, 0x70, 0x85, 0x75, 0xFB, 0x5A, 0xE7, 0x30, 0x6A, 
    0xCA, 0xFD, 0x3F, 0xD1, 0xD2, 0xF8, 0x7D, 0x23, 0xD4, 0x88, 
    0x9F, 0x7E, 0x0E, 0xDA, 0xE1, 0x7F, 0x23, 0x03, 0x00, 0x23, 
    0x6F, 0x71, 0x52, 0xAD, 0xC1, 0xEC, 0x24, 0xB0, 0xC8, 0x16, 
    0x16, 0x38, 0x24, 0x8A, 0x97, 0x70, 0x14, 0x1A, 0xFF, 0x23, 
    0x54, 0x3A, 0x80, 0x86, 0x54, 0xDA, 0xC2, 0x78, 0xBF, 0x2C, 
    0x45, 0x35, 0xA1, 0xC8, 0xC8, 0x91, 0xC7, 0x81, 0x09, 0x61, 
    0x35, 0x1E, 0x01, 0xA9, 0xCC, 0x21, 0x00, 0xB6, 0xC3, 0x72, 
    0x54, 0x66, 0x4A, 0x1F, 0xEB, 0x07, 0x5B, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
const int ginPubkeyDEV2DerLen = sizeof(ginPubkeyDEV2Der);













/* TODO: Debug only */
const uint8_t ginPubkeyDevKindDer[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xC2, 
    0xC9, 0x03, 0x30, 0xBD, 0x06, 0xAE, 0x23, 0x5F, 0xCF, 0x6E, 
    0xD4, 0x36, 0x17, 0x05, 0x53, 0x67, 0x00, 0xDC, 0x3D, 0xC0, 
    0xD7, 0x23, 0x59, 0xC4, 0x50, 0x8B, 0xEA, 0xB3, 0x3F, 0xAD, 
    0x96, 0x64, 0x33, 0xC2, 0x90, 0x96, 0x19, 0x43, 0xB0, 0x1D, 
    0xB6, 0x45, 0xDC, 0x05, 0x70, 0xF1, 0xBD, 0x73, 0xBC, 0x5E, 
    0x67, 0x5B, 0xF5, 0x14, 0xD1, 0xA1, 0x7C, 0xCF, 0x62, 0x8A, 
    0x0A, 0xAB, 0x66, 0xEA, 0x00, 0x78, 0x8D, 0x26, 0xE5, 0x6C, 
    0x96, 0x43, 0x52, 0x5B, 0xCC, 0x84, 0x04, 0xA2, 0x1A, 0x70, 
    0x94, 0x1A, 0x0D, 0xA5, 0x1D, 0xFD, 0xCC, 0x0E, 0xD8, 0x8C, 
    0x14, 0xE7, 0x86, 0xCB, 0x65, 0xE6, 0xBC, 0x12, 0xC3, 0x2D, 
    0x95, 0x4F, 0xDE, 0xE5, 0x3C, 0x80, 0x32, 0xDF, 0x4D, 0xF0, 
    0xAA, 0x5B, 0xE2, 0x56, 0xCD, 0x3A, 0xA8, 0x10, 0x16, 0x6B, 
    0x10, 0xAE, 0x48, 0x3D, 0x06, 0xE2, 0xFB, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
const int ginPubkeyDevKindDerLen = sizeof( ginPubkeyDevKindDer );











/* TODO: Debug only */
const uint8_t ginPrikeyDevKindDer[] =
{
    0x30, 0x82, 0x02, 0x78, 0x02, 0x01, 0x00, 0x30, 0x0D, 0x06, 
    0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 
    0x05, 0x00, 0x04, 0x82, 0x02, 0x62, 0x30, 0x82, 0x02, 0x5E, 
    0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xC2, 0xC9, 0x03, 
    0x30, 0xBD, 0x06, 0xAE, 0x23, 0x5F, 0xCF, 0x6E, 0xD4, 0x36, 
    0x17, 0x05, 0x53, 0x67, 0x00, 0xDC, 0x3D, 0xC0, 0xD7, 0x23, 
    0x59, 0xC4, 0x50, 0x8B, 0xEA, 0xB3, 0x3F, 0xAD, 0x96, 0x64, 
    0x33, 0xC2, 0x90, 0x96, 0x19, 0x43, 0xB0, 0x1D, 0xB6, 0x45, 
    0xDC, 0x05, 0x70, 0xF1, 0xBD, 0x73, 0xBC, 0x5E, 0x67, 0x5B, 
    0xF5, 0x14, 0xD1, 0xA1, 0x7C, 0xCF, 0x62, 0x8A, 0x0A, 0xAB, 
    0x66, 0xEA, 0x00, 0x78, 0x8D, 0x26, 0xE5, 0x6C, 0x96, 0x43, 
    0x52, 0x5B, 0xCC, 0x84, 0x04, 0xA2, 0x1A, 0x70, 0x94, 0x1A, 
    0x0D, 0xA5, 0x1D, 0xFD, 0xCC, 0x0E, 0xD8, 0x8C, 0x14, 0xE7, 
    0x86, 0xCB, 0x65, 0xE6, 0xBC, 0x12, 0xC3, 0x2D, 0x95, 0x4F, 
    0xDE, 0xE5, 0x3C, 0x80, 0x32, 0xDF, 0x4D, 0xF0, 0xAA, 0x5B, 
    0xE2, 0x56, 0xCD, 0x3A, 0xA8, 0x10, 0x16, 0x6B, 0x10, 0xAE, 
    0x48, 0x3D, 0x06, 0xE2, 0xFB, 0x02, 0x03, 0x01, 0x00, 0x01, 
    0x02, 0x81, 0x81, 0x00, 0xB1, 0x0C, 0x26, 0xD3, 0x39, 0x2D, 
    0x86, 0x40, 0xEF, 0x08, 0x4B, 0xD3, 0xA5, 0xEA, 0x9C, 0xD3, 
    0xA7, 0x2F, 0x58, 0x00, 0xE5, 0x74, 0x67, 0x54, 0x63, 0xA5, 
    0x56, 0xD8, 0x18, 0xDF, 0x8B, 0x77, 0xC4, 0x52, 0x6B, 0xCA, 
    0x22, 0x34, 0x8A, 0x9A, 0xDD, 0x16, 0xDD, 0x3E, 0xD4, 0xCF, 
    0x49, 0x5B, 0x8B, 0x84, 0x2F, 0x46, 0xC1, 0x85, 0xED, 0xCB, 
    0x71, 0x7E, 0x51, 0xBE, 0x5E, 0x7C, 0xBE, 0x37, 0x6F, 0x19, 
    0x1C, 0x61, 0xE6, 0xDE, 0x39, 0xC4, 0x6B, 0xEC, 0x5A, 0x6C, 
    0x49, 0xF1, 0xD4, 0xED, 0x39, 0x7D, 0x87, 0x73, 0x51, 0xE2, 
    0x79, 0x01, 0x92, 0x93, 0x82, 0x91, 0xC0, 0x89, 0xF9, 0xEC, 
    0x1A, 0xC0, 0x8C, 0x3F, 0x6D, 0x34, 0x08, 0x18, 0x37, 0xFA, 
    0x9E, 0xC4, 0x02, 0x10, 0xBD, 0x23, 0xAD, 0x9F, 0x58, 0x36, 
    0x07, 0xF1, 0x6A, 0xF2, 0x9E, 0x6E, 0x7F, 0x5C, 0x4C, 0x5B, 
    0x9F, 0x19, 0x02, 0x41, 0x00, 0xFB, 0xF1, 0x67, 0x0D, 0x20, 
    0x4A, 0x13, 0xEC, 0x62, 0xF7, 0xAD, 0xEA, 0xC8, 0x14, 0x8B, 
    0xCF, 0xF3, 0x80, 0x09, 0xD5, 0x6C, 0x4A, 0xA0, 0xEB, 0xF7, 
    0x64, 0xFE, 0xD7, 0x28, 0x87, 0x9D, 0xB9, 0xEA, 0xF5, 0x98, 
    0x91, 0xF9, 0xE1, 0xD1, 0x28, 0x1A, 0xED, 0x90, 0x18, 0xBE, 
    0xC5, 0xF8, 0x69, 0x42, 0x62, 0xB0, 0xFD, 0xF4, 0x03, 0x13, 
    0xF6, 0x56, 0xE0, 0x66, 0xC3, 0xC8, 0x74, 0x27, 0x0F, 0x02, 
    0x41, 0x00, 0xC5, 0xEB, 0xFC, 0x49, 0x93, 0x2E, 0xDA, 0x2F, 
    0x2C, 0xA7, 0x56, 0x53, 0x46, 0x03, 0x4D, 0x4A, 0x65, 0x84, 
    0x1A, 0xCE, 0x1B, 0x12, 0x04, 0x5E, 0x0C, 0xC7, 0xBD, 0x67, 
    0x0E, 0xCE, 0xB9, 0xDA, 0x53, 0x43, 0x02, 0xE7, 0xD2, 0xE8, 
    0xF8, 0xE5, 0x6F, 0x77, 0xBB, 0x31, 0x3F, 0xA1, 0x13, 0x38, 
    0x34, 0x68, 0xE2, 0x65, 0x0D, 0x95, 0x8A, 0x3C, 0x71, 0xFD, 
    0x8D, 0x58, 0x3C, 0x23, 0x65, 0x55, 0x02, 0x41, 0x00, 0xF0, 
    0x59, 0x3E, 0x9F, 0x36, 0x13, 0x95, 0x68, 0x28, 0x8D, 0xF2, 
    0x5B, 0x8D, 0x9E, 0x94, 0x36, 0xC1, 0x2C, 0x7F, 0xB5, 0x1C, 
    0x07, 0x21, 0xF7, 0x9A, 0x5E, 0xBE, 0x03, 0x12, 0x86, 0x36, 
    0x01, 0x1C, 0x56, 0x49, 0xC3, 0xD0, 0xE8, 0x0D, 0xB9, 0xBD, 
    0xDE, 0xAE, 0x5D, 0xFC, 0xF9, 0x2D, 0xDF, 0x74, 0xD6, 0x63, 
    0xD5, 0x11, 0x49, 0x32, 0x6D, 0x0A, 0x3A, 0x2F, 0xAF, 0x4C, 
    0xF7, 0x0A, 0x1F, 0x02, 0x40, 0x57, 0xAA, 0xE8, 0xCC, 0x1A, 
    0x32, 0xA5, 0x98, 0x41, 0xBA, 0x39, 0x65, 0x97, 0x5C, 0x7D, 
    0x0E, 0xD8, 0x1A, 0x84, 0xD9, 0x08, 0x6A, 0x99, 0x9E, 0xA3, 
    0x20, 0x92, 0x47, 0xA4, 0xEA, 0x72, 0xAE, 0x3F, 0x35, 0x2E, 
    0x83, 0x4B, 0x0B, 0xC0, 0xBB, 0xB7, 0xFD, 0x1B, 0xE0, 0x7B, 
    0xA3, 0xB8, 0x64, 0xAB, 0xEA, 0x4D, 0x65, 0x9F, 0x77, 0xBE, 
    0x8C, 0x79, 0x9D, 0x6A, 0xCF, 0x4B, 0x8C, 0x0E, 0x25, 0x02, 
    0x41, 0x00, 0xE9, 0x35, 0xAA, 0xC6, 0x6D, 0xCC, 0x06, 0x90, 
    0xA4, 0x5D, 0xB7, 0x76, 0xDC, 0xD7, 0x2F, 0xD1, 0xF8, 0xB5, 
    0x1E, 0x6C, 0x3C, 0xEC, 0x30, 0x59, 0x2B, 0xFC, 0x15, 0xB6, 
    0x4A, 0x36, 0xC9, 0x52, 0x0B, 0x20, 0xD2, 0x88, 0x90, 0xF3, 
    0xD0, 0x1A, 0x3E, 0x23, 0xC7, 0x19, 0x33, 0xA8, 0xC3, 0x18, 
    0xB9, 0x4C, 0x1B, 0xE0, 0xEC, 0x35, 0x8D, 0xAA, 0xA9, 0x51, 
    0x2A, 0xF1, 0xB0, 0x7A, 0x64, 0x04
};
const int ginPrikeyDevKindDerLen = sizeof(ginPrikeyDevKindDer);

/* /home/lf/Documents/dev/crypto/CERT/weiliang/global_prikey.der, 1024-bit */
const uint8_t ginPrikeyGlobalDer[] =
{
    0x30, 0x82, 0x02, 0x77, 0x02, 0x01, 0x00, 0x30, 0x0D, 0x06, 
    0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 
    0x05, 0x00, 0x04, 0x82, 0x02, 0x61, 0x30, 0x82, 0x02, 0x5D, 
    0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0x91, 0xFE, 0xAF, 
    0x0E, 0xF7, 0x84, 0x2C, 0x5F, 0xDC, 0xA5, 0x5F, 0xD2, 0xF5, 
    0x68, 0x6D, 0x6D, 0x71, 0xCC, 0xBA, 0xC0, 0xB3, 0x98, 0x60, 
    0xBA, 0x63, 0x06, 0xFF, 0x5E, 0x58, 0xC7, 0x75, 0x4F, 0x0B, 
    0x49, 0x06, 0x57, 0x1F, 0x66, 0x29, 0x9E, 0x57, 0xE7, 0xF4, 
    0xFF, 0xB5, 0xAD, 0x90, 0xE0, 0xE7, 0x0C, 0x6F, 0x5F, 0x76, 
    0x5F, 0x33, 0x72, 0x5C, 0xB7, 0x27, 0xFF, 0x22, 0xE4, 0x1E, 
    0x54, 0x17, 0xBA, 0xB3, 0x58, 0x0E, 0x50, 0xEF, 0x88, 0x3F, 
    0xE1, 0x37, 0x18, 0x37, 0xDE, 0xEF, 0x79, 0x88, 0xFE, 0xC3, 
    0xC4, 0x4D, 0x3A, 0x61, 0x3A, 0xC9, 0x9B, 0x75, 0x95, 0xB1, 
    0x25, 0x2A, 0xB7, 0x32, 0xD6, 0x29, 0xBB, 0x1F, 0xA4, 0x88, 
    0xA6, 0xE1, 0x21, 0xA7, 0x55, 0x37, 0x50, 0x85, 0x67, 0x0D, 
    0x02, 0x9C, 0x17, 0xF4, 0x4B, 0x12, 0xAC, 0xB0, 0xB2, 0x13, 
    0x88, 0xCF, 0x3E, 0x3E, 0x75, 0x02, 0x03, 0x01, 0x00, 0x01, 
    0x02, 0x81, 0x81, 0x00, 0x82, 0x75, 0x86, 0x73, 0x26, 0x65, 
    0x1A, 0xCE, 0x12, 0xBA, 0x48, 0x95, 0x97, 0xAC, 0x58, 0x54, 
    0x7B, 0x63, 0x89, 0xE2, 0x56, 0xDA, 0x2C, 0x5D, 0x81, 0xCE, 
    0x27, 0xF6, 0x63, 0xF8, 0xE0, 0x6F, 0xE0, 0xD2, 0xC1, 0x72, 
    0xAA, 0xE1, 0x3D, 0x01, 0x7C, 0xE6, 0x0E, 0xFD, 0x4D, 0x98, 
    0xAA, 0xA4, 0xFE, 0x72, 0x13, 0x37, 0xC7, 0xEC, 0x2C, 0x69, 
    0xDE, 0x45, 0xE0, 0xDC, 0xD1, 0x8D, 0xED, 0x36, 0xB7, 0x5D, 
    0xB0, 0x37, 0x34, 0x7D, 0x02, 0x6E, 0x67, 0x04, 0x55, 0x71, 
    0x39, 0xE5, 0x42, 0x7B, 0x27, 0x26, 0x82, 0x9B, 0x35, 0x66, 
    0x67, 0x9C, 0x34, 0x1B, 0x9F, 0x84, 0x76, 0x0E, 0x06, 0x84, 
    0xC3, 0x30, 0x2F, 0xD6, 0x07, 0xED, 0xF4, 0x3B, 0xE1, 0xCE, 
    0xD3, 0xA9, 0x4E, 0xF4, 0x7C, 0x59, 0xDD, 0x28, 0x6F, 0x16, 
    0x74, 0x78, 0x64, 0xD4, 0x14, 0x95, 0x1E, 0xF5, 0xC5, 0x75, 
    0x9D, 0x45, 0x02, 0x41, 0x00, 0xD1, 0x5D, 0x3E, 0xE8, 0xC6, 
    0x27, 0x78, 0x79, 0x57, 0x24, 0x25, 0x1C, 0xD2, 0x10, 0xAE, 
    0x86, 0x2B, 0x4E, 0x96, 0x46, 0x0C, 0x1A, 0x19, 0x7E, 0xA9, 
    0x5E, 0x32, 0x99, 0x02, 0xEE, 0x3D, 0xF8, 0x3A, 0x56, 0x6C, 
    0xA4, 0x18, 0xB5, 0xDF, 0xC1, 0xCA, 0x3F, 0x51, 0x58, 0xC2, 
    0x86, 0x82, 0x08, 0xA7, 0xFB, 0xED, 0x14, 0x2A, 0xCB, 0x55, 
    0x68, 0x32, 0xDB, 0x81, 0x8E, 0xB1, 0xAA, 0x14, 0x9F, 0x02, 
    0x41, 0x00, 0xB2, 0x83, 0xDF, 0x46, 0x8D, 0x03, 0xAB, 0xAE, 
    0xE9, 0x7E, 0xF6, 0xDD, 0x30, 0xAA, 0x50, 0xAB, 0x12, 0xF2, 
    0xB9, 0x57, 0xCD, 0xCB, 0x36, 0xA7, 0x3E, 0xC3, 0xE8, 0xF3, 
    0x22, 0xA5, 0xE2, 0x89, 0x10, 0x79, 0xE2, 0xA1, 0x57, 0xA2, 
    0xEA, 0x9D, 0x58, 0xAF, 0xFF, 0x63, 0x8F, 0x90, 0x89, 0x83, 
    0xF9, 0x16, 0x50, 0x87, 0x3F, 0xBC, 0x78, 0x26, 0x5E, 0xFF, 
    0x51, 0x53, 0x37, 0x14, 0x60, 0x6B, 0x02, 0x41, 0x00, 0xA3, 
    0x29, 0xFB, 0x74, 0x27, 0xB1, 0xED, 0x27, 0x0B, 0xAD, 0xA4, 
    0xAA, 0xC0, 0x5F, 0xB5, 0xD3, 0xE4, 0x7E, 0x5B, 0x88, 0xFD, 
    0xB7, 0x7D, 0x75, 0x04, 0x03, 0xE0, 0x84, 0xF5, 0x0E, 0xBD, 
    0x06, 0xEE, 0x58, 0x1A, 0x55, 0x0C, 0xD8, 0xF9, 0x28, 0x2B, 
    0x39, 0x8B, 0x69, 0x14, 0x39, 0x05, 0xB0, 0x3F, 0x52, 0x8B, 
    0xE2, 0x72, 0xB3, 0x82, 0xBD, 0x31, 0x1D, 0x76, 0xEB, 0xC1, 
    0x34, 0x5E, 0xDB, 0x02, 0x40, 0x1B, 0x14, 0xB2, 0x4C, 0x6B, 
    0x9C, 0x00, 0xF1, 0x79, 0xEE, 0x8E, 0xD8, 0xA3, 0x47, 0x53, 
    0x11, 0x80, 0xC0, 0x5D, 0xA9, 0x9A, 0x48, 0x97, 0xB3, 0xEB, 
    0x6B, 0xA0, 0xED, 0x31, 0x76, 0x64, 0xD5, 0x52, 0x30, 0x8B, 
    0x56, 0xDA, 0x8A, 0x96, 0x78, 0xE4, 0x39, 0x5E, 0xCE, 0xE5, 
    0xBA, 0x91, 0x81, 0xF9, 0xC9, 0x8E, 0xD1, 0xD5, 0xB3, 0x6B, 
    0xAE, 0xB6, 0x7E, 0x99, 0x01, 0xBC, 0xBB, 0x54, 0x9D, 0x02, 
    0x40, 0x28, 0xBC, 0xAE, 0x7A, 0xD1, 0x93, 0x68, 0xB2, 0x89, 
    0xBB, 0x3E, 0xEC, 0xF6, 0x40, 0x14, 0x28, 0xF1, 0x07, 0x7E, 
    0x02, 0xEF, 0xC9, 0xF9, 0x13, 0x20, 0xC2, 0x2F, 0xDD, 0x89, 
    0x7C, 0x53, 0xB3, 0x89, 0x15, 0xD9, 0x2E, 0x5E, 0x65, 0xBE, 
    0x44, 0x6B, 0xB8, 0xEE, 0xCB, 0x71, 0xA9, 0x7B, 0x7A, 0x1E, 
    0x5F, 0x1C, 0x0C, 0x2B, 0xDD, 0x81, 0xBD, 0x67, 0x0A, 0xCE, 
    0xA5, 0x43, 0x61, 0x6B, 0x22
};
const int ginPrikeyGlobalDerLen = sizeof(ginPrikeyGlobalDer);
