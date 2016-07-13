/*
 * netConfig.c
 *
 * Create on: 2016-07-05
 *
 *    Author: feiguoyou@le.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pcap.h>
#include <unistd.h>
#include <pthread.h>
#include "airconfig.h"
#include "halHeader.h"
#include "netConfig.h"

#pragma pack(1)
typedef struct frame_control
{
	u_int16_t ver :2; /* protocol version*/
	u_int16_t type :2; /*frame type field (Management,Control,Data)*/
	u_int16_t subtype :4; /* frame subtype*/

	u_int16_t toDS :1; /* frame coming from Distribution system */
	u_int16_t fromDS :1; /*frame coming from Distribution system */
	u_int16_t moreFrag :1; /* More fragments?*/
	u_int16_t retry :1; /*was this frame retransmitted*/

	u_int16_t powMgt :1; /*Power Management*/
	u_int16_t moreDate :1; /*More Date*/
	u_int16_t protectedData :1; /*Protected Data*/
	u_int16_t order :1; /*Order*/
} frame_control;

struct ieee80211_radiotap_header {
	u_int8_t it_version;
	u_int8_t it_pad;
	u_int16_t it_len;
	u_int32_t it_present;
	u_int64_t MAC_timestamp;
	u_int8_t flags;
	u_int8_t dataRate;
	u_int16_t channelfrequency;
	u_int16_t channelType;
	int ssiSignal :8;
	int ssiNoise :8;
};

struct ieee80211_head {
	frame_control fc; // Frame Control Field: 0x0842
	u_int16_t wi_duration; // .000 0000 0000 0000 = Duration: 0 microseconds
	u_int8_t wi_add1[6]; // Receiver address: Broadcast (ff:ff:ff:ff:ff:ff)
	u_int8_t wi_add2[6]; // BSS Id: Tp-LinkT_db:00:be (e4:d3:32:db:00:be)
	u_int8_t wi_add3[6]; // Source address: 58:20:b1:79:e8:5f (58:20:b1:79:e8:5f)
	u_int16_t wi_fs; // Fragment number: 0 & Sequence number: 1636
	u_int32_t wi_check; // Frame check sequence: 0x6fbcb262 [correct]. 这个的值在数据的后面
	u_int64_t wi_ccmp; // CCMP parameters
};
#pragma pack()

#define CFGS_STATE_RUN      0x0001
#define CFGS_STATE_LOCK     0x0002
#define CFGS_STATE_GETAP    0x0004

typedef struct {
	const char *ifname;
	pcap_t *device;
	int count;
	uint16_t state;
	uint16_t base;
    int nowChannel;
    time_t lockTime;
	target_item_t item;
	ap_passport_t account;
	int channel_locked[MAX_CHANNEL_CARE];
} airConfig_t;

static airConfig_t sNetConfig = {"wlan0"};

static uint16_t getState(uint16_t mask);
static u_int16_t setState(uint16_t state, int flag);
static int getChByFr(u_int16_t fr);
static void setChannel(int channel);
static void *pcapTask(void *data);
static void *switchChannelTask(void *data);
static void capCallBk(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet);

int netConfigStart(void)
{
	pcap_t *device;
	pthread_t id1, id2;
	char errlog[PCAP_ERRBUF_SIZE];

    if(getState(CFGS_STATE_RUN)) {
        return 0;
    }
	printf("success: device: %s\n", sNetConfig.ifname);
	/* open a device, wait until a packet arrives */
	device = pcap_open_live(sNetConfig.ifname, BUFSIZ, 1, 0, errlog);
	if (!device) {
		printf("error: pcap_open_live('%s'): %s\n", sNetConfig.ifname, errlog);
		return -1;
	}
    sNetConfig.device = device;
    setState(CFGS_STATE_RUN, 1);
	pthread_create(&id2, NULL, pcapTask, NULL);
	pthread_create(&id1, NULL, switchChannelTask, NULL);
	return 0;
}

int netConfigCheck(void)
{
    return !getState(CFGS_STATE_GETAP);
}

static void capCallBk(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet)
{
	target_item_t item;
	int i, channel, ret;
	struct ieee80211_radiotap_header *rh = (struct ieee80211_radiotap_header *) packet;
	struct ieee80211_head *ih = (struct ieee80211_head *) (packet + rh->it_len);

	if (ih->fc.type != 2 || ih->fc.subtype != 0) {
		return;
	}
	channel = getChByFr(rh->channelfrequency);
	item.data = pkthdr->len - rh->it_len - sizeof(*ih);
	memcpy(item.mac_dst, ih->wi_add1, sizeof(item.mac_dst));
	memcpy(item.mac_src, ih->wi_add3, sizeof(item.mac_src));
	memcpy(item.mac_bssid, ih->wi_add2, sizeof(item.mac_bssid));
#if 0
    {
        u_char *ptr;
        ptr =item.mac_dst;
        printf("DA: %02X:%02X:%02X:%02X:%02X:%02X, ", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
        ptr =item.mac_bssid;
        printf("BS: %02X:%02X:%02X:%02X:%02X:%02X, ", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
        ptr =item.mac_src;
        printf("SA: %02X:%02X:%02X:%02X:%02X:%02X, ", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
        printf("%03d\n",item.data);
    }
#endif
	if (!(sNetConfig.state & CFGS_STATE_LOCK)) {
		ret = airconfig_do_sync(&item, channel, sNetConfig.channel_locked, (uint16_t *) &sNetConfig.base);
		if (ret == AIRCONFIG_NW_STATE_CHANNEL_LOCKED) {
            sNetConfig.item = item;
            sNetConfig.lockTime = time(NULL);
            setState(CFGS_STATE_LOCK, 1);
			printf("bssid lock!! base = %d, channel: ", sNetConfig.base);
			for (i = 0; i < MAX_CHANNEL_CARE; i++) {
				if (sNetConfig.channel_locked[i] == 0) {
					break;
				}
				printf("%d, ", sNetConfig.channel_locked[i]);
			}
			printf("\n");
		}
	} else if(time(NULL) - sNetConfig.lockTime < CHANNEL_LOCK_TIMEOUT) {
		for (i = 0; i < MAX_CHANNEL_CARE; i++) {
			if (sNetConfig.channel_locked[i] == channel) {
				break;
			}
			if (sNetConfig.channel_locked[i] == 0) {
				return;
			}
		}
        if(memcmp(item.mac_src, sNetConfig.item.mac_src, sizeof(item.mac_src))) {
            return;
        }
		ret = airconfig_get_info(item.data, sNetConfig.base, &sNetConfig.account, NULL, 0);
		if (ret) {
			printf("ssid[%s] passwd[%s]\r\n", sNetConfig.account.ssid, sNetConfig.account.psk);
            setState(CFGS_STATE_GETAP, 1);
            setState(CFGS_STATE_RUN, 0);
		}
    } else {
        setState(CFGS_STATE_LOCK, 0);
        printf("Timeout!!! Try again.\n");
    }
}

static void *pcapTask(void *data)
{
	/* wait loop forever */
	pcap_loop(sNetConfig.device, -1, capCallBk, NULL);
	pcap_close(sNetConfig.device);
}

static void setChannel(int channel)
{
	char cmd[64];

    if(sNetConfig.nowChannel != channel) {
        snprintf(cmd, sizeof(cmd), "iwconfig %s channel %d", sNetConfig.ifname, channel);
        system(cmd);
        sNetConfig.nowChannel = channel;
        /*printf("%s\n", cmd);*/
    }
}

static void *switchChannelTask(void *data)
{
	int i;

	while(getState(CFGS_STATE_RUN))
	{
		if (!getState(CFGS_STATE_LOCK)) {
			for (i = 1; i < 14; i++) {
				setChannel(i);
                usleep(1000 * 50);
			}
		} else {
			for (i = 0; i < MAX_CHANNEL_CARE; i++) {
				if (sNetConfig.channel_locked[i]) {
					setChannel(sNetConfig.channel_locked[i]);
                    usleep(1000 * 50);
				} else if (sNetConfig.channel_locked[i] == 0) {
					break;
				}
			}
		}
	}
}

static u_int16_t getState(uint16_t mask)
{
    return (sNetConfig.state & mask);
}

static u_int16_t setState(uint16_t state, int flag)
{
    if(flag == 0) {
        if((sNetConfig.state & CFGS_STATE_RUN) && (state & CFGS_STATE_RUN)) {
            pcap_breakloop(sNetConfig.device);
        }
        sNetConfig.state &= ~state;
    } else {
        if(!(sNetConfig.state & CFGS_STATE_RUN) && (state & CFGS_STATE_RUN)) {
            sNetConfig.state = CFGS_STATE_RUN;
        } else {
            sNetConfig.state |= state;
        }
    }
    return sNetConfig.state;
}

static int getChByFr(u_int16_t fr)
{
	int i, ch = 0;
    static const int chTab[] = {0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472};

    for(i = 0; i < (int)(sizeof(chTab) / sizeof(chTab[0])); i++) {
        if(chTab[i] == fr) {
            ch = i;
            break;
        }
    }
	return ch;
}

