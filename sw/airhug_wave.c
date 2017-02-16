/*
 * airhug_wave.c
 *
 * Create on: 2017-01-20
 *
 *    Author: feiguoyou@hotmail.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h> 
#include <arpa/inet.h>
#include "rc4Wrapper.h"
#include "airhug_wave.h"

#define SYN_DATA1   (0x46)
#define SYN_DATA2   (0x5a)
#define SYN_BASE    (0x20)
#define SYN_OFFSET  (0x08)

static uint8_t calcrc(const uint8_t *ptr, uint32_t len);

static inline int airhug_send(int fd, uint8_t data[4]) 
{
    struct sockaddr_in addr;
    const char *pdata = (char *)&addr - (fd & 0x17); // any data

    addr.sin_family = AF_INET;
    addr.sin_port = htons(10703);
    addr.sin_addr.s_addr = htonl(0xe8 << 24 | data[0] << 16 | data[1] << 8 | data[2]);

#if 1
    printf("%02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
#endif
    return !(sendto(fd, pdata, data[3], 0, (struct sockaddr*)&(addr), sizeof(addr)) == data[3]);
}

int airhug_wave(const uint8_t *data, uint16_t len, void (*delayms)(uint16_t ms))
{
    uint32_t i;
    int ret = -1, j, sockfd;
    uint8_t one[4], ttl = 1;
    uint8_t sync0[4] = {SYN_DATA1, SYN_DATA2, 0, SYN_BASE};
    uint8_t sync1[4] = {SYN_DATA1, SYN_DATA2, 0, SYN_BASE + SYN_OFFSET};

    if(!data || len <= 0 || !delayms) {
        goto out;
    }
    sync0[2] = len;
    sync1[2] = calcrc(data, len);
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("socket failed!\n");
        goto out;
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        printf("setsockopt failed!\n");
        goto out;
    }
#ifdef BIND_DEV
#include <net/if.h>
    struct ifreq ifr; 
    strncpy(ifr.ifr_name, BIND_DEV, IFNAMSIZ);
    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0) {
        printf("setsockopt SO_BINDTODEVICE failed, %s\n", strerror(errno));
        goto out;
    }
#endif
    for(i = 0; i < len; ) {
        if(airhug_send(sockfd, sync0)) {
            goto out;
        }
        if(airhug_send(sockfd, sync1)) {
            goto out;
        }
        delayms(10);
        for(j = 0; j < 2; j++) {
            one[3] = SYN_BASE + i;
            one[1] = data[i++];
            one[2] = data[i++];
            one[0] = (one[1] + one[2]) & 0x7f;
            if(airhug_send(sockfd, one)) {
                goto out;
            }
        }
    }
    ret = 0;
out:
    if(sockfd > 0) {
        close(sockfd);
    }

    return ret;
}

/*
 *   -----------------------------------------------------
 *  |   ver  |  len1  |     ssid     |  len2  |   passwd  |
 *   -----------------------------------------------------
 *  | 1bytes | 1bytes |  len1 bytes  | 1bytes | len2 bytes|
 *   -----------------------------------------------------
 */
int airhug_wave_ext(const char *ssid, const char *passwd, void (*delayms)(uint16_t ms))
{
    uint8_t s[256];
    uint16_t len = 0;
    uint8_t data[AIRHUG_MAX_LEN];
	static const char *key = "09n899uh39nhh9q34kk";

    if(!ssid || !passwd) {
        return -1;
    }
    printf("airConfig airhug_wave : ssid '%s', passwd '%s'\n", ssid, passwd);
    // version
    data[len++] = AIRHUG_VERSION;
    if(len + strlen(ssid) > AIRHUG_MAX_LEN) {
        return -1;
    }
    // ssid
    data[len++] = strlen(ssid);
    strcpy((char *)&data[len], ssid);
    len += strlen(ssid);
    if(len + strlen(passwd) > AIRHUG_MAX_LEN) {
        return -1;
    }
    // passwd
    data[len++] = strlen(passwd);
    strcpy((char *)&data[len], passwd);
    len += strlen(passwd);
    // rc4
    rc4_init(s, (uint8_t *)key, strlen(key));
    rc4_crypt(s, data, len);
    return airhug_wave(data, len, delayms);
}

static uint8_t calcrc(const uint8_t *ptr, uint32_t len)
{
    uint8_t i;
    uint8_t crc = 0;

    while(len--)
    {
        crc ^= *ptr++;
        for(i = 0;i < 8;i++) {
            if(crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else  {
                crc >>= 1;
            }
        }
    }
    return crc;
}
