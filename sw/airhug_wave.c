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
#include "leconfig.h"
#include "airhug_wave.h"

#define SYN_DATA1   (0x46)
#define SYN_DATA2   (0x45)
#define SYN_BASE    (0x20)
#define SYN_OFFSET  (0x08)


static uint8_t calcrc(uint8_t *ptr, uint32_t len);

static inline int airhug_send(int fd, uint8_t data[4]) 
{
    struct sockaddr_in addr;
    const char *pdata = (char *)&addr - (fd & 0x17); // any data

    addr.sin_family = AF_INET;
    addr.sin_port = htons(10703);
    addr.sin_addr.s_addr = htonl(0xe8 << 24 | data[0] << 16 | data[1] << 8 | data[2]);

    return !(sendto(fd, pdata, data[3], 0, (struct sockaddr*)&(addr), sizeof(addr)) == data[3]);
}

int airhug_wave(char *ssid, char *passwd, void (*delayms)(uint16_t ms))
{
    int ret = -1, j, sockfd;
    uint8_t i, n, one[4], data[128], ttl = 1;
    uint8_t sync0[4] = {SYN_DATA1, SYN_DATA2, 0, SYN_BASE};
    uint8_t sync1[4] = {SYN_DATA1, SYN_DATA2, 0, SYN_BASE + SYN_OFFSET};

    if(!ssid || !passwd || !delayms) {
        goto out;
    }
    n = strlen(ssid) + strlen(passwd);
    if(n <= 0 || n > 32 * 2) {
        goto out;
    }
    sync0[2] = ++n;
    data[0] = (uint8_t)strlen(ssid);
    memcpy(&data[1], ssid, strlen(ssid));
    memcpy(&data[1 + strlen(ssid)], passwd, strlen(passwd));
    sync1[2] = calcrc(data, n);
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        LELOGE("socket failed!\n");
        goto out;
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        LELOGE("setsockopt failed!\n");
        goto out;
    }
    for(i = 0; i < n; ) {
        if(airhug_send(sockfd, sync0)) {
            goto out;
        }
        if(airhug_send(sockfd, sync1)) {
            goto out;
        }
        delayms(10);
        for(j = 0; j < 2; j++) {
            one[3] = SYN_BASE + i;
            one[0] = data[i++];
            one[1] = data[i++];
            one[2] = one[0] + one[1];
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

static uint8_t calcrc(uint8_t *ptr, uint32_t len)
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
