#include "leconfig.h"
#include "utility.h"
#include "airconfig.h"
#include "io.h"
#include "data.h"
#include "aesWrapper.h"
// #define AIRCONFIG_DEBUG_LEVEL1
// #define AIRCONFIG_DEBUG_BASIC
// #define AIRCONFIG_DEBUG_VERBOSE

#define MAX_ITEM_SET 8 /* 7 is enough, item num of one node */
#define MAX_COUNT_SYNC 4 /* performance adjust, the shoot count for one item */
#define MAX_NODE_SET 16 /* performance adjust, node num of the set*/
#define MAX_HEADER_FLAG 8
#define MAX_COUNT_HEADER 2 /* performance adjust, the shoot count for one item */

// #define MAX_HEADER_MAGIC 4
// #define MAX_HEADER_PREFIX 4
#define MAX_INFO_LEN (64/4) + 1 // 32 + 32 + 1(rand) bytes for psk
#define MAX_SSID 32
enum {
    AIRCONFIG_STATE_GET_HEADER,
    AIRCONFIG_STATE_GET_DATA
};

typedef struct {
    target_item_t item[MAX_ITEM_SET];
    uint8_t count[MAX_ITEM_SET];
    uint8_t channel_care[MAX_CHANNEL_CARE][2];
    uint16_t base;
}target_node_t;

typedef struct {
    uint8_t flag;
    uint8_t reserved;
    uint8_t sequence[2];
    uint8_t data[4];
}aiconfig_info_t;

typedef struct {
    uint8_t data[128];
    uint8_t flag[128];
}aiconfig_note_t;

aiconfig_note_t gin_temp_data;

static uint8_t gin_airconfig_state = AIRCONFIG_STATE_GET_HEADER;
/* row flags for header, col[0] is the flags col[1] is the current counts */
static uint8_t gin_airconfig_flag_header[MAX_HEADER_FLAG];
static uint8_t gin_ssid_len;
static uint8_t gin_ssid_crc;
static uint8_t gin_psk_len;
static uint8_t gin_head_crc;
static uint8_t gin_info_idx, gin_info_block_remain, gin_info_block_total, gin_one_remain, gin_spencial_total;
static aiconfig_info_t gin_info_tmp;
static aiconfig_info_t gin_info[MAX_INFO_LEN];
static uint8_t ginSSIDOptimized[MAX_SSID];


static target_node_t gin_node_set[MAX_NODE_SET];

int airconfig_reset() {
    gin_airconfig_state = AIRCONFIG_STATE_GET_HEADER;
    gin_ssid_len = 0;
    gin_ssid_crc = 0;
    gin_psk_len = 0;
    gin_head_crc = 0;
    gin_info_idx = 0;
    gin_info_block_remain = 0;
    gin_one_remain = 0;
    gin_spencial_total = 0;
    memset(&gin_info_tmp, 0, sizeof(aiconfig_info_t));
    memset(gin_info, 0, sizeof(gin_info));
    memset(gin_node_set, 0, sizeof(gin_node_set));
    memset(gin_airconfig_flag_header, 0, sizeof(gin_airconfig_flag_header));
    memset(ginSSIDOptimized, 0, sizeof(ginSSIDOptimized));
    return 0;
}

int isHeaderReady(void) {
    int i = 0;
    uint8_t buff[3];
    for (i = 0; i < MAX_HEADER_FLAG; i++) {
        if (0 == gin_airconfig_flag_header[i]) {
            return 0;
        }
    }
    
    LELOG("HeaderReady");
    buff[0] = gin_ssid_crc;
    buff[1] = gin_ssid_len;
    buff[2] = gin_psk_len;
    if (gin_head_crc != crc8(buff, 3)) {
      LELOG("isHeaderReady crc error [%d][%d]", gin_ssid_len, gin_psk_len);
      airconfig_reset();
      return 0;
    }
    return 1;
}

// static int inner_airconfig_reset_data(aiconfig_info_t *info) {
//     int tmp_remain = 0, i = 0;
//     if (!info) {
//         return tmp_remain;
//     }
//     for (i = 0; i < sizeof(info->data); i++) {
//         if (info->data[i]) {
//             info->data[i] = 0;
//             tmp_remain++;
//         }
//     }
//     // with out the seq flags, only reset the data part.
//     info[gin_info_idx].flag &= 0x3;
//     return tmp_remain;
// }

static target_node_t *inner_airconfig_shoot_to_node(const target_item_t *item) {
    int i = 0;

#ifdef AIRCONFIG_DEBUG_BASIC
    if (((item->mac_src[0] == 0x54) &&
        (item->mac_src[1] == 0xE4) &&
        (item->mac_src[2] == 0x3A) &&
        (item->mac_src[3] == 0x4A) &&
        (item->mac_src[4] == 0x45) &&
        (item->mac_src[5] == 0xE6)) || 
        ((item->mac_src[0] == 0xB8) &&
        (item->mac_src[1] == 0x86) &&
        (item->mac_src[2] == 0x87) &&
        (item->mac_src[3] == 0xDB) &&
        (item->mac_src[4] == 0x05) &&
        (item->mac_src[5] == 0x2C)) ||
        ((item->mac_src[0] == 0x3C) &&
        (item->mac_src[1] == 0x15) &&
        (item->mac_src[2] == 0xC2) &&
        (item->mac_src[3] == 0xE3) &&
        (item->mac_src[4] == 0x81) &&
        (item->mac_src[5] == 0x2A))/* || 1*/) {
        LELOG("catched->TRU:%02X:%02X:%02X:%02X:%02X:%02X--SRC:%02X:%02X:%02X:%02X:%02X:%02X--DST:%02X:%02X:%02X:%02X:%02X:%02X---%d",
            item->mac_bssid[0], item->mac_bssid[1], item->mac_bssid[2], item->mac_bssid[3], item->mac_bssid[4], item->mac_bssid[5],
            item->mac_src[0], item->mac_src[1], item->mac_src[2], item->mac_src[3], item->mac_src[4], item->mac_src[5],
            item->mac_dst[0], item->mac_dst[1], item->mac_dst[2], item->mac_dst[3], item->mac_dst[4], item->mac_dst[5],
            item->data);
    }
#endif

    // for none-empty node
    for (i = 0; i < MAX_NODE_SET; i++) {
        uint16_t mid_data = gin_node_set[i].item[MAX_ITEM_SET / 2].data;
        // LELOG("[Prov] mid_data [%d] ", mid_data);
        if (0 < mid_data) {
            int16_t diff = item->data - mid_data;
            // on the left or right of the middle
            if ((-(MAX_ITEM_SET / 2) < diff && 0 >= diff) || (MAX_ITEM_SET / 2 > diff && 0 <= diff)) {
                // shoot on the current node
                if (0 == memcmp(item->mac_src, gin_node_set[i].item[MAX_ITEM_SET / 2].mac_src, sizeof(item->mac_src))) {
                    // reset if no balance.
                    if (++gin_node_set[i].count[MAX_ITEM_SET / 2 + diff] >= MAX_COUNT_SYNC * 2) {
                        memset(&(gin_node_set[i]), 0, sizeof(target_node_t));
                        return NULL;
                    }

                    memcpy(&(gin_node_set[i].item[MAX_ITEM_SET / 2 + diff]), item, sizeof(target_item_t));
                    if (gin_node_set[i].base > item->data) {
                        gin_node_set[i].base = item->data;
                    }

#ifdef AIRCONFIG_DEBUG_BASIC
                    if (((item->mac_src[0] == 0x54) &&
                        (item->mac_src[1] == 0xE4) &&
                        (item->mac_src[2] == 0x3A) &&
                        (item->mac_src[3] == 0x4A) &&
                        (item->mac_src[4] == 0x45) &&
                        (item->mac_src[5] == 0xE6)) || 
                        ((item->mac_src[0] == 0xB8) &&
                        (item->mac_src[1] == 0x86) &&
                        (item->mac_src[2] == 0x87) &&
                        (item->mac_src[3] == 0xDB) &&
                        (item->mac_src[4] == 0x05) &&
                        (item->mac_src[5] == 0x2C)) ||
                        ((item->mac_src[0] == 0x3C) &&
                        (item->mac_src[1] == 0x15) &&
                        (item->mac_src[2] == 0xC2) &&
                        (item->mac_src[3] == 0xE3) &&
                        (item->mac_src[4] == 0x81) &&
                        (item->mac_src[5] == 0x2A))/* || 1*/) {

                        int m, n;
                        LELOG("[Prov] ALREADY => data[%d][%02x:%02x:%02x:%02x:%02x:%02x]", item->data,
                            item->mac_src[0],
                            item->mac_src[1],
                            item->mac_src[2],
                            item->mac_src[3],
                            item->mac_src[4],
                            item->mac_src[5]);
                        for (m = 0; m < MAX_NODE_SET; m++) {
                            for (n = 0; n < MAX_ITEM_SET; n++) {
                                LELOG("[Prov] ALREADY [%d] item[%d] count[%d]data[%d][%02x:%02x:%02x:%02x:%02x:%02x] ", 
                                    m, n, gin_node_set[m].count[n], gin_node_set[m].item[n].data,
                                    gin_node_set[m].item[n].mac_src[0],
                                    gin_node_set[m].item[n].mac_src[1],
                                    gin_node_set[m].item[n].mac_src[2],
                                    gin_node_set[m].item[n].mac_src[3],
                                    gin_node_set[m].item[n].mac_src[4],
                                    gin_node_set[m].item[n].mac_src[5]);
                            }
                        }
                    }
#endif
                    return &gin_node_set[i];
                }
                else {
                    // not the same src
#ifdef AIRCONFIG_DEBUG_VERBOSE
                    LELOG("[Prov] NOMATCHED 1 => data[%d][%02x:%02x:%02x:%02x:%02x:%02x]", item->data,
                        item->mac_src[0],
                        item->mac_src[1],
                        item->mac_src[2],
                        item->mac_src[3],
                        item->mac_src[4],
                        item->mac_src[5]);
#endif
                }
            }
            // shoot out of the current node, so reset the current node
            else {
                // only reset the same src matched node
                if (0 == memcmp(item->mac_src, gin_node_set[i].item[MAX_ITEM_SET / 2].mac_src, sizeof(item->mac_src))) {
#ifdef AIRCONFIG_DEBUG_VERBOSE
                    int m, n;
                    LELOG("[Prov] RESET => gin_node_set[%d] ", i);
                    for (m = 0; m < MAX_NODE_SET; m++) {
                        for (n = 0; n < MAX_ITEM_SET; n++) {
                            LELOG("gin_node_set[%d] item[%d] count[%d]data[%d][%02x:%02x:%02x:%02x:%02x:%02x] ", 
                                m, n, gin_node_set[m].count[n], gin_node_set[m].item[n].data,
                                gin_node_set[m].item[n].mac_src[0],
                                gin_node_set[m].item[n].mac_src[1],
                                gin_node_set[m].item[n].mac_src[2],
                                gin_node_set[m].item[n].mac_src[3],
                                gin_node_set[m].item[n].mac_src[4],
                                gin_node_set[m].item[n].mac_src[5]);
                        }
                    }
#endif
                    memset(&(gin_node_set[i]), 0, sizeof(target_node_t));
                    return NULL;
                }
                else{
#ifdef AIRCONFIG_DEBUG_VERBOSE
                    LELOG("[Prov] NOMATCHED 2 => data[%d][%02x:%02x:%02x:%02x:%02x:%02x]", item->data,
                        item->mac_src[0],
                        item->mac_src[1],
                        item->mac_src[2],
                        item->mac_src[3],
                        item->mac_src[4],
                        item->mac_src[5]);                    
#endif
                }

            }
        }
    }

    // looking up for an empty node
    for (i = 0; i < MAX_NODE_SET; i++) {
        uint16_t mid_data = gin_node_set[i].item[MAX_ITEM_SET / 2].data;
        if (0 == mid_data) {
            memcpy(&(gin_node_set[i].item[MAX_ITEM_SET / 2]), item, sizeof(target_item_t));
            gin_node_set[i].count[MAX_ITEM_SET / 2] = 1;
            gin_node_set[i].base = item->data;
#ifdef AIRCONFIG_DEBUG_VERBOSE
            LELOG("[Prov] GOT EMPTY 2 => data[%d][%02x:%02x:%02x:%02x:%02x:%02x]", item->data,
                item->mac_src[0],
                item->mac_src[1],
                item->mac_src[2],
                item->mac_src[3],
                item->mac_src[4],
                item->mac_src[5]);  
            int m, n;
            for (m = 0; m < MAX_NODE_SET; m++) {
                for (n = 0; n < MAX_ITEM_SET; n++) {
                    LELOG("gin_node_set[%d] item[%d] count[%d]data[%d][%02x:%02x:%02x:%02x:%02x:%02x] ", 
                        m, n, gin_node_set[m].count[n], gin_node_set[m].item[n].data,
                        gin_node_set[m].item[n].mac_src[0],
                        gin_node_set[m].item[n].mac_src[1],
                        gin_node_set[m].item[n].mac_src[2],
                        gin_node_set[m].item[n].mac_src[3],
                        gin_node_set[m].item[n].mac_src[4],
                        gin_node_set[m].item[n].mac_src[5]);
                }
            }                
#endif
            return &gin_node_set[i];
        }
    }

    return NULL;
}

int airconfig_do_sync(const target_item_t *item, int channel, int channel_locked[MAX_CHANNEL_CARE], uint16_t *base) {
    target_node_t *node = NULL;
    // PrivateCfg cfg;

    if (!item) {
        return AIRCONFIG_NW_STATE_ERROR_PARAM;
    }

    if (0 == memcmp(item->mac_src, item->mac_bssid, 6)) {
        return AIRCONFIG_NW_STATE_NONE;
    }

    // flashReadPrivateCfg(&cfg);
    // if (1 == cfg.data.nwCfg.configStatus) {
    //     LELOG("LOCKED from flash");
    //     return AIRCONFIG_NW_STATE_CHANNEL_LOCKED;
    // }


    // TODO: check if the target is full, figure out if need to enlarge
    int m = 0;
    for (m = 0; m < MAX_NODE_SET; m++) {
        if (!gin_node_set[m].item[4].data) {
            break;
        }
    }
    if (m == MAX_NODE_SET) {
        for (m = 0; m < MAX_NODE_SET; m++) {
            LELOG("gin_node_set[%d] src[%02x:%02x:%02x:%02x:%02x:%02x] bssid[%02x:%02x:%02x:%02x:%02x:%02x] [%d]\n", 
                m,
                gin_node_set[m].item[4].mac_src[0], 
                gin_node_set[m].item[4].mac_src[1],
                gin_node_set[m].item[4].mac_src[2],
                gin_node_set[m].item[4].mac_src[3],
                gin_node_set[m].item[4].mac_src[4],
                gin_node_set[m].item[4].mac_src[5],
                gin_node_set[m].item[4].mac_bssid[0], 
                gin_node_set[m].item[4].mac_bssid[1],
                gin_node_set[m].item[4].mac_bssid[2],
                gin_node_set[m].item[4].mac_bssid[3],
                gin_node_set[m].item[4].mac_bssid[4],
                gin_node_set[m].item[4].mac_bssid[5],
                gin_node_set[m].item[4].data);
        }
        // TODO: enlarge the target space
        memset(gin_node_set, 0, sizeof(gin_node_set));
    }

    node = inner_airconfig_shoot_to_node(item);
    if (node) {
        int i = 0, fulled = 0;
        for (i = 0; i < MAX_CHANNEL_CARE; i++) {
            if (0 == node->channel_care[i][0]) {
                node->channel_care[i][0] = (uint8_t)channel;
                node->channel_care[i][1]++;
                break;
            }
            else if (channel == node->channel_care[i][0]) {
                node->channel_care[i][1]++;
                break;
            }
        }
        i = 0;
        while (MAX_COUNT_SYNC <= node->count[i++]) {
            LELOG("channel counts [%d]", i);
            break;
        }
        if(i<=MAX_ITEM_SET / 2) {
            int j = 0;
            while((j < MAX_ITEM_SET / 2) && (MAX_COUNT_SYNC <= node->count[i+j]) && (node->base+j == node->item[i+j].data)) {
                j++;
            }
            if(j>=MAX_ITEM_SET / 2) {
                fulled = MAX_ITEM_SET / 2;
                // right data
            } else {
                return AIRCONFIG_NW_STATE_NONE;
            }
        } else {
            // not right data
            return AIRCONFIG_NW_STATE_NONE;
        }
        // check if full filed the target 
        if (fulled >= MAX_ITEM_SET / 2) {
            int valid_channel = 0, valid_counts = 0;
            for (i = 0; i < MAX_CHANNEL_CARE; i++) {
                LELOG("channel [%d] counts [%d]", node->channel_care[i][0], node->channel_care[i][1]);
                if (node->channel_care[i][1] > valid_counts) {
                    valid_channel = node->channel_care[i][0];
                    valid_counts = node->channel_care[i][1];
                }
                node->channel_care[i][0] = 0;
                node->channel_care[i][1] = 0;
            }
            node->channel_care[0][0] = valid_channel;
            node->channel_care[0][1] = valid_counts;
            channel_locked[0] = valid_channel;
            for (i = 0; i < MAX_CHANNEL_CARE; i++) {
                LELOG("merge to one channel [%d] counts [%d]", node->channel_care[i][0], node->channel_care[i][1]);
            }
            for (i = 0; i < MAX_ITEM_SET; i++) {
                LELOG("node[%i]data[%d][%02x:%02x:%02x:%02x:%02x:%02x]=>[%02x:%02x:%02x:%02x:%02x:%02x]=>[%02x:%02x:%02x:%02x:%02x:%02x]count[%d]", i, node->item[i].data, 
                    node->item[i].mac_src[0],
                    node->item[i].mac_src[1],
                    node->item[i].mac_src[2],
                    node->item[i].mac_src[3],
                    node->item[i].mac_src[4],
                    node->item[i].mac_src[5], 
                    node->item[i].mac_bssid[0],
                    node->item[i].mac_bssid[1],
                    node->item[i].mac_bssid[2],
                    node->item[i].mac_bssid[3],
                    node->item[i].mac_bssid[4],
                    node->item[i].mac_bssid[5], 
                    node->item[i].mac_dst[0],
                    node->item[i].mac_dst[1],
                    node->item[i].mac_dst[2],
                    node->item[i].mac_dst[3],
                    node->item[i].mac_dst[4],
                    node->item[i].mac_dst[5],                                         
                    node->count[i]);
            }
            *base = node->base - 1;
            for(i=0; i<128; i++) {
                gin_temp_data.flag[i] = 0;
                gin_temp_data.data[i] = 0;
            }
            return AIRCONFIG_NW_STATE_CHANNEL_LOCKED;
        }

    }

    return AIRCONFIG_NW_STATE_NONE;
}

void logprint(void)
{
    int last_bytes;
    int i, j;
    int m = 0, n = 0, x = 0;
    uint8_t bufCheck[1+32+1+32] = {0};
    // recover the data
    gin_spencial_total = 1 + gin_psk_len + 1 + gin_ssid_len;
    gin_info_block_total = (gin_spencial_total - 1)/4 + 1;
    last_bytes = gin_spencial_total%4 ? gin_spencial_total%4 : 4;

    LELOG("DONE ok gin_info_block_total[%d] gin_psk_len[%d] gin_ssid_len[%d] last_bytes[%d]", 
            gin_info_block_total, gin_psk_len, gin_ssid_len, last_bytes);

    for (i = 0; i < gin_info_block_total; i++) {
        int bytes = ((i+1) == gin_info_block_total) ? last_bytes : 4;
        for (j = 0; j < bytes; j++) {
            if (i*4 + j <= gin_psk_len) {
                // passport->psk[m++] = gin_info[i].data[j];
                if (i != 0 || j != 0)
                    m++;
            } else if (i*4 + j > (1 + gin_psk_len)) {
                // passport->ssid[n++] = gin_info[i].data[j];
                n++;
            }
            bufCheck[x++] = gin_info[i].data[j];

        }
    }
    LEPRINTF("bufCheck => m[%d] n[%d]\r\n", m, n);
    for (i = 0; i < (1 + m + 1 + n); i++) {
        LEPRINTF("[0x%02x], [%c]\r\n", bufCheck[i], bufCheck[i]);
    }
    LEPRINTF("<=== \r\n");

    if (bufCheck[0] == crc8(&bufCheck[1], m+1+n)) {
        LELOG("bufCheck OK => ");
    } else {
        LELOG("bufCheck NOT OK => ");
    }
}

static void airconfig_bin2str(uint8_t* src, uint16_t src_len, uint8_t* dst, uint16_t dst_len) {
    uint16_t i, temp_len;
    // uint8_t half_data;

    temp_len = (src_len*3)/8;

    if(temp_len != dst_len) {
        LELOGE("lelink_bin2str dst len not right [%d] [%d]", temp_len, dst_len);
    }

    for(i=0; i<src_len; i++) {
        src[i] = src[i] & 0x07;
    }

    //APPLOGE("lelink_bin2str src=[%d] dst=[%d]\r\n ", src_len,dst_len);

    for(i=0; i<dst_len; i++) {
      uint8_t a = (i*8%3); 
      uint8_t c = i*8/3;
      //APPLOGE("lelink_bin2str a=[%d] c=[%d]\r\n ", a,c);
      if(a==0) {
        dst[i] = src[c] << 5;
        dst[i] |= (src[c+1] << 2);
        dst[i] |= (src[c+2] >> 1);
        //APPLOGE("lelink_bin2str a=0 [%d] [%c]\r\n ", i, dst[i]);
      } else if(a==1) {
        dst[i] = src[c] << 6;
        dst[i] |= (src[c+1] << 3);
        dst[i] |= (src[c+2]);
        //APPLOGE("lelink_bin2str a=1 [%d] [%c]\r\n ", i, dst[i]);
      } else {
        dst[i] = src[c] << 7;
        dst[i] |= (src[c+1] << 4);
        dst[i] |= (src[c+2] << 1);
        dst[i] |= (src[c+3] >> 2);
        //APPLOGE("lelink_bin2str a=2 [%d] [%c]\r\n ", i, dst[i]);
      }
    }
    //APPLOGE("lelink_bin2str return\r\n ");
}

int airconfig_get_info(int len, int base, ap_passport_t *passport, const char *currSSID, int currSSIDLen) {
// PrivateCfg cfg;
#ifdef AIRCONFIG_DEBUG_LEVEL1
    LELOG("len[%d] base[%d] data[%d][0x%04x]", len, base, len - base, len - base);
#endif
// comming from the same src. but maybe multi-channel or multi-bssid
// for Air Kiss, it presumes comming from the same bssid

// for magic and prefix
// static uint16_t header[MAX_HEADER_MAGIC + MAX_HEADER_PREFIX] = { 0 };

    uint16_t data = (len - base);

    if (AIRCONFIG_STATE_GET_HEADER == gin_airconfig_state) {
        // 5 bit
        uint8_t hdata = (uint8_t)((data & 0x01F0) >> 4);
        // 4 bit
        uint8_t ldata = (uint8_t)(data & 0x000F);
        switch (hdata) {
            case 0x00: {
                gin_airconfig_flag_header[hdata]++;
                gin_head_crc |= (ldata << 4);
            }break;
            case 0x01: {
                gin_airconfig_flag_header[hdata]++;
                gin_head_crc |= ldata;
                //LELOG("0x01 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x02: {
                gin_airconfig_flag_header[hdata]++;
                gin_ssid_crc |= (ldata << 4);
                //LELOG("0x02 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x03: {
                gin_airconfig_flag_header[hdata]++;
                gin_ssid_crc |= ldata;
                //LELOG("0x03 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x04: {
                gin_airconfig_flag_header[hdata]++;
                gin_ssid_len |= (ldata << 4);
                //LELOG("0x04 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x05: {
                gin_airconfig_flag_header[hdata]++;
                gin_ssid_len |= ldata;
                //LELOG("0x05 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x06: {
                gin_airconfig_flag_header[hdata]++;
                gin_psk_len |= (ldata << 4);
                //LELOG("0x06 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            case 0x07: {
                gin_airconfig_flag_header[hdata]++;
                gin_psk_len |= ldata;
                //LELOG("0x07 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
            }break;
            default: {
                #if 0
                int i = 0;
                LEPRINTF("EXCEPTION AIRCONFIG_STATE_GET_HEADER [0x%04x] => \n", data);
                for (i = 0; i < MAX_HEADER_FLAG; i++) {
                LEPRINTF("idx[%d] counts[%d]\n", i, gin_airconfig_flag_header[i]);
                }
                LEPRINTF("<= EXCEPTION HEADER \n");
                #endif
            }break;
        }
        if ((isHeaderReady())) {
            int i = 0;
            gin_airconfig_state = AIRCONFIG_STATE_GET_DATA;
            if (gin_ssid_len >= 128) {
                gin_ssid_len -= 128;
            }

            LEPRINTF("DONE HEADER => \r\n");
            for (i = 0; i < MAX_HEADER_FLAG; i++) {
                LEPRINTF("idx[%d] counts[%d]\r\n", i, gin_airconfig_flag_header[i]);
            }
            LEPRINTF("<= DONE HEADER \r\n");

            gin_spencial_total = 1 + gin_psk_len + 1 + gin_ssid_len;
            gin_info_block_remain = gin_info_block_total = (gin_spencial_total - 1)/4 + 1;
            // gin_one_remain = gin_psk_len%4 ? gin_psk_len%4 : 4;
            #ifdef AIRCONFIG_DEBUG_LEVEL1
            LELOG("DONE AIRCONFIG_STATE_GET_HEADER ssid[0x%02x][%d] psk[0x%02x][%d] remain block[%d] ", 
                gin_ssid_crc, gin_ssid_len, gin_head_crc, gin_psk_len, gin_info_block_remain);
            #endif
        }
    } else if (AIRCONFIG_STATE_GET_DATA == gin_airconfig_state) {
        int total_bytes = 1 + gin_psk_len + 1 + gin_ssid_len;
        int total_bin;
        uint8_t index = (data & 0x3F8) >> 3;
        uint8_t bufCheck[1+32+1+32] = {0};
        uint8_t i;

        if((total_bytes*8)%3 > 0) {
            total_bin = (total_bytes*8)/3;
            total_bin++;
        } else {
            total_bin = (total_bytes*8)/3;
        }

        //LELOG("got index[%d] data[%d] total_bytes[%d] total_bin[%d]!!!!!", index, data, total_bytes, total_bin);

        if(index < total_bin) {
            gin_temp_data.data[index] = data;
            gin_temp_data.flag[index]++;
        }

        // optimized
        if (!ginSSIDOptimized[0] && gin_ssid_crc == crc8((const uint8_t *)currSSID, currSSIDLen)) {
            memcpy(ginSSIDOptimized, currSSID, gin_ssid_len > MAX_SSID ? MAX_SSID : gin_ssid_len);
            LELOG("DONE PRE CAP SSID & CACHED[%s]", currSSID);
            // index = ((1 + gin_psk_len + 1)*8)%3;
            // switch (index) {
            //     case 0:
            //     break;
            //     case 1:
            //         {

            //         }
            //     break;
            //     case 2:
            //     break;
            // }

            // for (kk = 0; kk < tmpCurrLen; kk++) {
            //     gin_temp_data.data[2 + gin_psk_len + kk] = ginSSIDOptimized[kk];
            //     gin_temp_data.flag[2 + gin_psk_len + kk]++;
            // }

            // gin_temp_data.data[2 + gin_psk_len + kk] = '\0';
            // restBinLen = total_bin - total_bytes*8
            total_bytes = 1 + gin_psk_len + 1;
            if((total_bytes*8)%3 > 0) {
                total_bin = (total_bytes*8)/3;
                total_bin++;
            } else {
                total_bin = (total_bytes*8)/3;
            }
        }

        for(i=0; i<total_bin; i++) {
            if(gin_temp_data.flag[i] == 0)
                break;
        }


        if(i >= total_bin) {
            memset(bufCheck, 0, 66);
            if (ginSSIDOptimized[0]) {
                memcpy(&bufCheck[gin_psk_len+2], ginSSIDOptimized, gin_ssid_len > MAX_SSID ? MAX_SSID : gin_ssid_len);
                LELOG("PRE ssid is copied");
            }
            airconfig_bin2str(gin_temp_data.data, total_bin, bufCheck, total_bytes);
            LELOG("got complete [%s][%s]!!!!!",  &bufCheck[1], &bufCheck[gin_psk_len+2]);
            if(bufCheck[0] == (crc8(&bufCheck[1], gin_psk_len+gin_ssid_len+1))) {
                LELOG("got right data in TOTAL");
                gin_info_block_remain = 0;
            } else {
                if (ginSSIDOptimized[0]) {
                    memcpy(&bufCheck[2 + gin_psk_len], ginSSIDOptimized, gin_ssid_len > MAX_SSID ? MAX_SSID : gin_ssid_len);
                    if(bufCheck[0] == (crc8(&bufCheck[1], gin_psk_len+gin_ssid_len+1))) {
                        LELOG("got right data in OPTIMIZED(ssid)");
                        gin_info_block_remain = 0;
                    }
                }
            }
            for(i=0; i<total_bin; i++) {
                gin_temp_data.flag[i] = 0;
                gin_temp_data.data[i] = 0;
            }
        }

        if (!gin_info_block_remain) {
            memcpy(passport->psk, &bufCheck[1], gin_psk_len);
            memcpy(passport->ssid, &bufCheck[gin_psk_len + 2], gin_ssid_len);
            passport->psk[gin_psk_len] = '\0';
            passport->ssid[gin_ssid_len] = '\0';

            LELOG("DONE passport =>");
            LELOG("passport->psk [%s]", passport->psk);
            LELOG("passport->ssid [%s]", passport->ssid);
            {
                PrivateCfg cfg;
                int ret = 0;
                lelinkStorageReadPrivateCfg(&cfg);
                LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                    cfg.data.nwCfg.config.ssid,
                    cfg.data.nwCfg.config.psk, 
                    cfg.data.nwCfg.configStatus);
                strcpy(cfg.data.nwCfg.config.ssid, passport->ssid);
                cfg.data.nwCfg.config.ssid_len = strlen(passport->ssid);
                cfg.data.nwCfg.config.ssid[cfg.data.nwCfg.config.ssid_len] = '\0';
                strcpy(cfg.data.nwCfg.config.psk, passport->psk);
                cfg.data.nwCfg.config.psk_len = strlen(passport->psk);
                cfg.data.nwCfg.config.psk[cfg.data.nwCfg.config.psk_len] = '\0';
                cfg.data.nwCfg.configStatus = 1;
                ret = lelinkStorageWritePrivateCfg(&cfg);
                LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
            }

            LELOG("SSID[%s] pwd[%s]", passport->ssid, passport->psk);
            LELOG("DONE psk ");
            return 1;
        }
    } else {
    // LELOG("EXCEPTION type [0x%04x] ", data);
    }


    return 0;
}

#include "network.h"

#define WIFICONFIG_LISTEN_PORT      (4911)
#define WIFICONFIG_MAGIC            (0x7689)
#define WIFICONFIG_SOFTAP_VER       (1)
//#define WIFICONFIG_CKSUM_LEN        ((uint32_t)&(((wificonfig_t *)0)->reserved))
#define WIFICONFIG_CKSUM_LEN        (2 + 32 + 32)

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t checksum;
    uint16_t reserved;
    uint8_t ssid[32];
    uint8_t wap2passwd[32];
} wificonfig_t;

int softApStart(void);
int softApCheck(void);
int softApStop(int success);

static void *ginApNetCtx = NULL;
static uint8_t ginSoftAPAESKey[AES_LEN] = {0};
/*
 * 功能: 启动softap
 *
 * 返回值: 
 *      0 表示启动AP成功
 *
 */
int softApStart(void)
{
    int ret;
    char ssid[32];
    char uuid[32] = {0};
    uint8_t mac[6] = {0};
    uint8_t retCRC8 = 0;
    char wpa2_passphrase[32] = "00000000";

    softApStop(0);
    if((ret = getTerminalUUID((uint8_t *)uuid, sizeof(uuid))) < 0) {
        LELOGE("getTerminalUUID ret[%d]", ret);
        goto out;
    }
    halGetMac(mac, 6);
    retCRC8 = crc8(mac, sizeof(mac));
    snprintf(ssid, sizeof(ssid), "lelink%03d%s%02X", WIFICONFIG_SOFTAP_VER, uuid, retCRC8);
    memset(ginSoftAPAESKey, 0, AES_LEN);
    if((ret = halSoftApStart(ssid, wpa2_passphrase, ginSoftAPAESKey, AES_LEN))) {
        LELOGE("halSoftApStart ret[%d]", ret);
        goto out;
    }
    ginApNetCtx = lelinkNwNew(NULL, 0, 4911, NULL);
    if(!ginApNetCtx) {
        LELOGE("New link");
        goto out;
    }
    return 0;
out:
    softApStop(0);
    return ret;
}

/*
 * 功能: 停止softap
 *
 * 返回值: 
 *      0 表示停止AP成功
 *
 */
int softApStop(int success)
{
    halSoftApStop(success);
    if(ginApNetCtx) {
        lelinkNwDelete(ginApNetCtx);
        ginApNetCtx = NULL;
    }
    return 0;
}

/*
 * 功能: 在softap模式下，接收AP的配置信息
 *
 * 返回值: 
 *      0 表示成功接收到AP的信息
 *
 */
int softApCheck(void)
{
    int ret;
    uint8_t sum;
    uint16_t port;
    char ipaddr[32];
    wificonfig_t wc;
    char buf[UDP_MTU];

    if(!ginApNetCtx) {
        return -1;
    }
    LELOG("Checking wifi configure...");
    ret = nwUDPRecvfrom(ginApNetCtx, (uint8_t *)buf, UDP_MTU, ipaddr, sizeof(ipaddr), &port);
    if(ret <= 0 ) {
        return -2;
    }
    LELOG("nwUDPRecvfrom ret = %d sizeof(wc)[%d]", ret, sizeof(wc));
    if(ret != ENC_SIZE(AES_LEN, sizeof(wc) + 1)) {
        LELOGE("wrong len = %d", ret);
        return -3;
    }

{
    uint8_t iv[AES_LEN] = { 0 };
    uint8_t key[AES_LEN] = { 0 };
    uint32_t decLen = ENC_SIZE(AES_LEN, sizeof(wc) + 1);
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    if (ginSoftAPAESKey[0]) {
        memcpy(key, ginSoftAPAESKey, AES_LEN);
    } else {
        memcpy(key, getPreSharedToken(), AES_LEN);
    }
    {
        extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
        uint8_t  hexStr[96] = {0};
        bytes2hexStr(key, AES_LEN, hexStr, sizeof(hexStr));
        LELOG("key[%s]", hexStr);
    }

    ret = aes(iv, 
        key, 
        (uint8_t *)&buf,
        &decLen, /* in-len/out-enc size */
        sizeof(wc),
        0);
    LELOG("dec ret[%d] [%d/%d]", ret, ENC_SIZE(AES_LEN, sizeof(wc) + 1), decLen);
    if (0 > ret) {
        return -4;
    }
    memcpy(&wc, buf, sizeof(wc));
}

    if(wc.magic != WIFICONFIG_MAGIC) {
        LELOGE("magic = %d", wc.magic);
        return -5;
    }
    sum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);
    if(wc.checksum != sum) {
        LELOGE("checksum = %d", wc.magic);
        return -6;
    }
    LELOG("Get ssid[%s] passwd[%s]", wc.ssid, wc.wap2passwd);
    {
        PrivateCfg cfg;
        lelinkStorageReadPrivateCfg(&cfg);
        LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                cfg.data.nwCfg.config.ssid,
                cfg.data.nwCfg.config.psk, 
                cfg.data.nwCfg.configStatus);
        strcpy((char *)cfg.data.nwCfg.config.ssid, (const char *)wc.ssid);
        cfg.data.nwCfg.config.ssid_len = strlen((const char *)wc.ssid);
        cfg.data.nwCfg.config.ssid[cfg.data.nwCfg.config.ssid_len] = '\0';
        strcpy((char *)cfg.data.nwCfg.config.psk, (const char *)wc.wap2passwd);
        cfg.data.nwCfg.config.psk_len = strlen((const char *)wc.wap2passwd);
        cfg.data.nwCfg.config.psk[cfg.data.nwCfg.config.psk_len] = '\0';
        cfg.data.nwCfg.configStatus = 1;
        ret = lelinkStorageWritePrivateCfg(&cfg);
        LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
    }
    return ret;
}

int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout, const char *aesKey)
{
    void *ctx;
    uint16_t port = 4911;
    int i, ret, count, delay = 1000; // ms
    char ipaddr[32] = "192.168.10.1";
    wificonfig_t wc = { WIFICONFIG_MAGIC, WIFICONFIG_SOFTAP_VER, 0 };
    uint8_t iv[AES_LEN] = { 0 };
    uint8_t key[AES_LEN] = { 0 };
    uint8_t beEncData[ENC_SIZE(AES_LEN, sizeof(wc) + 1)] = {0};
    uint32_t encLen = sizeof(wc);

    if (NULL == aesKey) {
        aesKey = "157e835e6c0bc55474abcd91e00e6979";
    }

    ctx  = lelinkNwNew(NULL, 0, 0, NULL);
    if(!ctx) {
        LELOGE("New link error");
        return -2;
    }
    count = timeout / delay + 1;
    strncpy((char *)wc.ssid, ssid, sizeof(wc.ssid));
    strncpy((char *)wc.wap2passwd, passwd, sizeof(wc.wap2passwd));
    wc.checksum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);


    memcpy(beEncData, &wc, sizeof(wc));
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    LELOG("AES %s", aesKey);
    hexStr2bytes(aesKey, key, AES_LEN);
    ret = aes(iv, 
        key, 
        (uint8_t *)&beEncData,
        &encLen, /* in-len/out-enc size */
        sizeof(wc),
        1);
    LELOG("enc ret[%d] [%d/%d] sizeof(wc)[%d]", ret, ENC_SIZE(AES_LEN, sizeof(wc) + 1), encLen, sizeof(wc));
    if (0 > ret) {
        lelinkNwDelete(ctx);
        return -3;
    }
#if 0
        ret = aes(iv, 
            key, 
            (uint8_t *)&beEncData,
            &encLen, /* in-len/out-enc size */
            sizeof(wc),
            0);
        LELOG("dec ret[%d]", ret);
        memcpy(&wc, beEncData, sizeof(wc));
#endif

    for( i = 0; i < count; i++ ) {
        LELOG("Send wifi configure, [%s:%s][%d]...", ssid, passwd, delay);
        halDelayms(delay);
        ret = nwUDPSendto(ctx, ipaddr, port, (uint8_t *)beEncData, encLen);
        if(ret <= 0 ) {
            LELOGE("nwUDPSendto ret = %d", ret);
        }
    }
    if(ctx) {
        lelinkNwDelete(ctx);
    }
    return 0;
}
