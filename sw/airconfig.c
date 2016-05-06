#include "leconfig.h"
#include "utility.h"
#include "airconfig.h"
#include "io.h"

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

static uint8_t gin_airconfig_state = AIRCONFIG_STATE_GET_HEADER;
/* row flags for header, col[0] is the flags col[1] is the current counts */
static uint8_t gin_airconfig_flag_header[MAX_HEADER_FLAG];
static uint8_t gin_ssid_len;
static uint8_t gin_ssid_crc;
static uint8_t gin_psk_len;
static uint8_t gin_psk_crc;
static uint8_t gin_info_idx, gin_info_block_remain, gin_info_block_total, gin_one_remain, gin_spencial_total;
static aiconfig_info_t gin_info_tmp;
static aiconfig_info_t gin_info[MAX_INFO_LEN];

static target_node_t gin_node_set[MAX_NODE_SET];

int airconfig_reset() {
    gin_airconfig_state = AIRCONFIG_STATE_GET_HEADER;
    gin_ssid_len = 0;
    gin_ssid_crc = 0;
    gin_psk_len = 0;
    gin_psk_crc = 0;
    gin_info_idx = 0;
    gin_info_block_remain = 0;
    gin_one_remain = 0;
    gin_spencial_total = 0;
    memset(&gin_info_tmp, 0, sizeof(aiconfig_info_t));
    memset(gin_info, 0, sizeof(gin_info));
    memset(gin_node_set, 0, sizeof(gin_node_set));
    memset(gin_airconfig_flag_header, 0, sizeof(gin_airconfig_flag_header));
    return 0;
}

int isHeaderReady(void) {
    int i = 0;
    for (i = 0; i < MAX_HEADER_FLAG; i++) {
        if (MAX_COUNT_HEADER >= gin_airconfig_flag_header[i]) {
            return 0;
        }
    }
    return 1;
}

static int inner_airconfig_reset_data(aiconfig_info_t *info) {
    int tmp_remain = 0, i = 0;
    if (!info) {
        return tmp_remain;
    }
    for (i = 0; i < sizeof(info->data); i++) {
        if (info->data[i]) {
            info->data[i] = 0;
            tmp_remain++;
        }
    }
    // with out the seq flags, only reset the data part.
    info[gin_info_idx].flag &= 0x3;
    return tmp_remain;
}

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
                    if (gin_node_set[i].base < item->data) {
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
        for (i = 0; i < MAX_ITEM_SET; i++) {
            if (MAX_COUNT_SYNC <= node->count[i]) {
                fulled++;
            }
        }

        // check if full filed the target 
        if (fulled >= MAX_ITEM_SET / 2) {
            int valid_channel = 0, valid_counts = 0;
            for (i = 0; i < MAX_CHANNEL_CARE; i++) {
                channel_locked[i] = node->channel_care[i][0];
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
            *base = node->base - MAX_ITEM_SET / 2;
            return AIRCONFIG_NW_STATE_CHANNEL_LOCKED;
        }

    }

    return AIRCONFIG_NW_STATE_NONE;
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
            // magic part start
        case 0x00: { // high 4 bits of ssid len
            if (gin_airconfig_flag_header[MAX_HEADER_FLAG-1]) { 
                gin_airconfig_flag_header[hdata]++;
                if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                    break;
                gin_ssid_len |= (ldata << 4);
                LELOG("0x00 counts[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
                // gin_airconfig_flag_header[hdata] = 1;
            }
        }break;
        case 0x01: { // low 4 bits of ssid len
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_ssid_len |= ldata;
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x01 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        case 0x02: { // high 4 bits of ssid crc
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_ssid_crc |= (ldata << 4);
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x02 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        case 0x03: { // low 4 bits of ssid crc
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_ssid_crc |= ldata;
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x03 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
            // magic part end
            // prefix part start
        case 0x04: { // high 4 bits of psk len
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_psk_len |= (ldata << 4);
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x04 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        case 0x05: { // low 4 bits of psk len
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_psk_len |= ldata;
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x05 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        case 0x06: { // high 4 bits of psk crc
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_psk_crc |= (ldata << 4);
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x06 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        case 0x07: { // low 4 bits of psk crc
            gin_airconfig_flag_header[hdata]++;
            if (MAX_COUNT_HEADER > gin_airconfig_flag_header[hdata] || MAX_COUNT_HEADER < gin_airconfig_flag_header[hdata]) 
                break;
            gin_psk_crc |= ldata;
            // gin_airconfig_flag_header[hdata] = 1;
            LELOG("0x07 flag_header[%d] [0x%x] ", gin_airconfig_flag_header[hdata], gin_ssid_len);
        }break;
        
        // prefix part end
        default: {
            int i = 0;
            LEPRINTF("EXCEPTION AIRCONFIG_STATE_GET_HEADER [0x%04x] => ", data);
            for (i = 0; i < MAX_HEADER_FLAG; i++) {
                LEPRINTF("idx[%d] counts[%d]", i, gin_airconfig_flag_header[i]);
            }
            LEPRINTF("<= EXCEPTION HEADER ");
        }break;
    }
        // if ((0xFF == gin_airconfig_flag_header)) {
        if ((isHeaderReady())) {
            int i = 0;
            gin_airconfig_state = AIRCONFIG_STATE_GET_DATA;
            if (gin_ssid_len >= 128) {
                gin_ssid_len -= 128;
            }

            LEPRINTF("DONE HEADER => ");
            for (i = 0; i < MAX_HEADER_FLAG; i++) {
                LEPRINTF("idx[%d] counts[%d]", i, gin_airconfig_flag_header[i]);
            }
            LEPRINTF("<= DONE HEADER ");

            // TODO:
#define AIRKISS
#ifdef AIRKISS
            gin_ssid_len = gin_ssid_len - gin_psk_len - 1;  
#endif
            gin_spencial_total = gin_psk_len + gin_ssid_len + 1;
            gin_info_block_remain = gin_info_block_total = (gin_spencial_total - 1)/4 + 1;
            // gin_one_remain = gin_psk_len%4 ? gin_psk_len%4 : 4;
            #ifdef AIRCONFIG_DEBUG_LEVEL1
            LELOG("DONE AIRCONFIG_STATE_GET_HEADER ssid[0x%02x][%d] psk[0x%02x][%d] remain block[%d] ", 
                gin_ssid_crc, gin_ssid_len, gin_psk_crc, gin_psk_len, gin_info_block_remain);
            #endif
        }
    }
    else if (AIRCONFIG_STATE_GET_DATA == gin_airconfig_state) {
        uint8_t ret_crc8 = 0;

        // PRE CAP the SSID
        if (currSSID && 0 < currSSIDLen) {
            uint8_t tmpCRC8 = crc8((const uint8_t *)currSSID, currSSIDLen);
            if (tmpCRC8 == gin_ssid_crc) {
                // int tmpLen = currSSIDLen > (sizeof(passport->ssid)-1) ? (sizeof(passport->ssid)-1) : currSSIDLen;
                // memcpy(passport->ssid, currSSID, tmpLen);
                // passport->ssid[tmpLen] = '\0';
                int ssidIdx = gin_psk_len%4 + 1;
                int i, j, k = 0;
                int startBlockIdx = gin_psk_len/4;
                int last_bytes = gin_spencial_total%4 ? gin_spencial_total%4 : 4;
                for (i = startBlockIdx; i < gin_info_block_total; i++) {
                    int bytes = ((i+1) == gin_info_block_total) ? last_bytes : 4;
                    for (j = ((i == startBlockIdx) ? ssidIdx : 0); j < bytes; j++) {
                        gin_info[i].data[j] = currSSID[k++];
                    }
                }
                
                if (gin_psk_len%4) {
                    gin_spencial_total = gin_psk_len + 1 + (gin_ssid_len >= (4 - ssidIdx) ? (4 - ssidIdx) : gin_ssid_len);
                } else {
                    gin_spencial_total = gin_psk_len;
                }
                gin_info_block_remain = (gin_spencial_total - 1)/4 + 1;
                // gin_one_remain = gin_psk_len%4 ? gin_psk_len%4 : 4;
                LELOG("DONE PRE SSID [%s] ssid[0x%02x][%d] psk[0x%02x][%d] remain block[%d] total[%d]", currSSID,
                    gin_ssid_crc, gin_ssid_len, gin_psk_crc, gin_psk_len, gin_info_block_remain, gin_spencial_total);
                gin_ssid_crc = 0;


                LELOG("DONE PRE CAP SSID => ");
                for (i = 0; i < gin_info_block_total; i++) {
                    LEPRINTF("idx[%d] => ", i);
                    for (j = 0; j < 4; j++) {
                        LEPRINTF("%02x ", gin_info[i].data[j]);
                    }
                    LEPRINTF("\r\n");
                }
                // LELOG("SSID[%s] pwd[%s]", passport->ssid, passport->psk);
                LELOG("DONE PRE CAP SSID ");
            }
        }

        // only for sequence
        // if (!(data & 0x100)) {
        if (0x3 != (0x3 & gin_info_tmp.flag)) {
        seqStart:
            if (data & 0x80) {
                if (!(gin_info_tmp.flag & 0x1)) {
                    gin_info_tmp.sequence[0] = 0x7F & data;
                    gin_info_tmp.flag |= 0x1;
                    #ifdef AIRCONFIG_DEBUG_LEVEL1
                    LELOG("START seq [0x%04x] seq[0]->[0x%02x] seq[1]->[0x%02x]", 
                        data, gin_info_tmp.sequence[0], gin_info_tmp.sequence[1]);
                    #endif
                }
                else if (!(gin_info_tmp.flag & 0x2)) {
                    // reset seq flags, impossible index
                    if (gin_info_block_total <= (0x7F & data)) {
                        gin_info_tmp.flag &= ~0x3;
                    #ifdef AIRCONFIG_DEBUG_LEVEL1
                        LELOG("EXCEPTION seq2 [0x%04x] seq[0]->[0x%02x] seq[1]->[0x%02x]", 
                            data, gin_info_tmp.sequence[0], gin_info_tmp.sequence[1]);
                        goto seqStart;
                    #endif
                    } else  { // normal seq id
                        gin_info_tmp.sequence[1] = 0x7F & data;
                        gin_info_tmp.flag |= 0x2;

                        // if matched the last one
                        if ((gin_info_block_total - 1) == gin_info_tmp.sequence[1]) {
                            gin_one_remain = gin_spencial_total%4 ? gin_spencial_total%4 : 4;
                        } else {
                            gin_one_remain = 4;
                        }
                        // reset data
                        inner_airconfig_reset_data(&gin_info_tmp);
                    #ifdef AIRCONFIG_DEBUG_LEVEL1
                        LELOG("DONE sequence flag[0x%02x] crc[0x%02x] seq[%d] gin_one_remain[%d]", 
                            gin_info_tmp.flag, gin_info_tmp.sequence[0], gin_info_tmp.sequence[1], gin_one_remain);
                    #endif
                    }
                } else { // reset all seq flags
                    // gin_info_tmp.flag &= ~0x3;
                    // if (gin_info_block_total <= data) {
                    //     gin_info_tmp.sequence[0] = 0x7F & data;
                    //     gin_info_tmp.flag |= 0x1;
                    // }
                    // LELOG("EXCEPTION sequence2 [0x%04x] [0x%02x] [0x%02x]", 
                    //     data, gin_info_tmp.sequence[0], gin_info_tmp.sequence[1]);
                }
            }
        // only for data
        } else {
        // } else if (0x3 == (0x3 & gin_info_tmp.flag)) {
#ifdef AIRCONFIG_DEBUG_LEVEL1
            // LELOG("GETTING data [0x%04x] ", data);
#endif
            // reset to new seq
            if (!(data & 0x100) && (0 < gin_one_remain)) {
                gin_info_tmp.flag &= ~0x2F;
                gin_one_remain = 4;
                LELOG("GOTO seqStart ");
                goto seqStart;
            }

            if (!(gin_info_tmp.flag & 0x4)) {
                gin_info_tmp.data[0] = 0xFF & data;
                gin_info_tmp.flag |= 0x4;
                gin_one_remain--;
            }
            else if (!(gin_info_tmp.flag & 0x8)) {
                gin_info_tmp.data[1] = 0xFF & data;
                gin_info_tmp.flag |= 0x8;
                gin_one_remain--;
            }
            else if (!(gin_info_tmp.flag & 0x10)) {
                gin_info_tmp.data[2] = 0xFF & data;
                gin_info_tmp.flag |= 0x10;
                gin_one_remain--;
            }
            else if (!(gin_info_tmp.flag & 0x20)) {
                gin_info_tmp.data[3] = 0xFF & data;
                gin_info_tmp.flag |= 0x20;
                gin_one_remain--;
            }
            else {
                //LELOG("EXCEPTION NOWAY data1 [0x%04x] ", data);
            }
        }


        // one has read completed
        if (!gin_one_remain) {
            int tmp_remain;
            // if it is the last one
            if ((gin_info_block_total - 1) == gin_info_tmp.sequence[1]) {
                tmp_remain = gin_spencial_total%4 ? gin_spencial_total%4 : 4;
                // LELOG("DONE last one ");
            } else {
                tmp_remain = 4;
            }
            ret_crc8 = crc8(&(gin_info_tmp.sequence[1]), tmp_remain + 1);
            #ifdef AIRCONFIG_DEBUG_LEVEL1
            LELOG("DONE one seq[%d][0x%02x, 0x%02x, 0x%02x, 0x%02x] crc8[0x%02x]rmt[0x%02x] ", 
                gin_info_tmp.sequence[1], 
                gin_info_tmp.data[0], 
                gin_info_tmp.data[1], 
                gin_info_tmp.data[2], 
                gin_info_tmp.data[3],
                ret_crc8, gin_info_tmp.sequence[0]);
            #endif
            gin_info_tmp.flag &= ~0x2F;
            gin_one_remain = 4;

            if ((ret_crc8 & 0x7F) == gin_info_tmp.sequence[0]) {
                if (memcmp(&gin_info[gin_info_tmp.sequence[1]], &gin_info_tmp, sizeof(aiconfig_info_t))) {
                    int tmpTotalBlocks = (gin_spencial_total - 1)/4 + 1;
                    if (tmpTotalBlocks > gin_info_tmp.sequence[1]) {
                        memcpy(&gin_info[gin_info_tmp.sequence[1]], &gin_info_tmp, sizeof(aiconfig_info_t));
                        gin_info_block_remain--;
                        #ifdef AIRCONFIG_DEBUG_LEVEL1
                        LELOG("DONE GOT [%d] still[%d] [0x%02x, 0x%02x, 0x%02x, 0x%02x]", 
                            gin_info_tmp.sequence[1], gin_info_block_remain,
                            gin_info[gin_info_tmp.sequence[1]].data[0],
                            gin_info[gin_info_tmp.sequence[1]].data[1],
                            gin_info[gin_info_tmp.sequence[1]].data[2],
                            gin_info[gin_info_tmp.sequence[1]].data[3]);
                        #endif
                    }
                } else {
                    #ifdef AIRCONFIG_DEBUG_LEVEL1
                    LELOG("REPEAT GOT [%d/%d]", gin_info_tmp.sequence[1], gin_info_block_remain);
                    #endif
                }
            }
        }

        if (!gin_info_block_remain) {
            int last_bytes;
            int i, j;
            int m = 0, n = 0;
            // recover the data
            gin_spencial_total = gin_psk_len + gin_ssid_len + 1;
            gin_info_block_total = (gin_spencial_total - 1)/4 + 1;
            last_bytes = gin_spencial_total%4 ? gin_spencial_total%4 : 4;

            LELOG("DONE ok gin_info_block_total[%d] gin_psk_len[%d] gin_ssid_len[%d] last_bytes[%d]", 
                gin_info_block_total, gin_psk_len, gin_ssid_len, last_bytes);

            for (i = 0; i < gin_info_block_total; i++) {
                int bytes = ((i+1) == gin_info_block_total) ? last_bytes : 4;
                for (j = 0; j < bytes; j++) {
                    if (i*4 + j < gin_psk_len) {
                        passport->psk[m++] = gin_info[i].data[j];
                    } else if (i*4 + j > gin_psk_len) {
                        passport->ssid[n++] = gin_info[i].data[j];
                    }
                }
            }
            passport->psk[m] = '\0';
            passport->ssid[n] = '\0';

            LELOG("DONE PASSPORT SSID => ");
            for (i = 0; i < gin_info_block_total; i++) {
                LEPRINTF("idx[%d] => ", i);
                for (j = 0; j < 4; j++) {
                    LEPRINTF("%02x ", gin_info[i].data[j]);
                }
                LEPRINTF("\r\n");
            }
                // LELOG("SSID[%s] pwd[%s]", passport->ssid, passport->psk);
            LELOG("DONE PASSPORT SSID");

            LELOG("DONE passport =>");
            for (i = 0; i < gin_info_block_total; i++) {
                for (j = 0; j < 4; j++) {
                    LEPRINTF("%c", gin_info[i].data[j]);
                }
            }
            LEPRINTF("\r\n");
            {
                PrivateCfg cfg;
                int ret = 0;
                // flashWritePrivateCfg(&cfg);
                // static int flag = 0;
                lelinkStorageReadPrivateCfg(&cfg);
                LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                    cfg.data.nwCfg.config.ssid,
                    cfg.data.nwCfg.config.psk, 
                    cfg.data.nwCfg.configStatus);
                // if (0 != cfg.data.nwCfg.configStatus) {
                    strcpy(cfg.data.nwCfg.config.ssid, passport->ssid);
                    strcpy(cfg.data.nwCfg.config.psk, passport->psk);
                    cfg.data.nwCfg.configStatus = 1;
                    ret = lelinkStorageWritePrivateCfg(&cfg);
                    LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);

                    // test only
                    ret = lelinkStorageReadPrivateCfg(&cfg);
                    LELOG("lelinkStorageReadPrivateCfg[%d] ssid[%s], psk[%s], configStatus[%d]", 
                        ret, 
                        cfg.data.nwCfg.config.ssid,
                        cfg.data.nwCfg.config.psk, 
                        cfg.data.nwCfg.configStatus);
                    if (cfg.csum != crc8((const uint8_t *)&(cfg.data), sizeof(cfg.data))) {
                        LELOG("lelinkStorageReadPrivateCfg csum failed");
                    }
                // }

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
int softApStarted(void)
{
    int ret;
    char ssid[32];
    uint16_t port;
    uint8_t sum;
    char ipaddr[32];
    wificonfig_t wc;
    char buf[UDP_MTU];
    void *ctx = NULL;
    char uuid[32] = {0};
    char wpa2_passphrase[32] = "00000000";

    if((ret = getTerminalUUID((uint8_t *)uuid, sizeof(uuid))) < 0) {
        LELOGE("getTerminalUUID ret[%d]", ret);
        goto out;
    }
    snprintf(ssid, sizeof(ssid), "-lelink%03d-%s", WIFICONFIG_VERSION, uuid);
    if((ret = halSoftApStart(ssid, wpa2_passphrase))) {
        LELOGE("halSoftApStart ret[%d]", ret);
        goto out;
    }
    ctx = lelinkNwNew(NULL, 0, 4911, NULL);
    if(!ctx) {
        LELOGE("New link");
        goto out;
    }
    while(1) {
        LELOG("Waitting wifi configure.");
        delayms(1000);
        ret = nwUDPRecvfrom(ctx, (uint8_t *)buf, UDP_MTU, ipaddr, sizeof(ipaddr), &port);
        if(ret > 0 ) {
            LELOG("nwUDPRecvfrom ret = %d", ret);
            if(ret != sizeof(wc)) {
                LELOGE("Wrong len = %d", ret);
                continue;
            }
            memcpy(&wc, buf, ret);
            if(wc.magic != WIFICONFIG_MAGIC) {
                LELOGE("magic = %d", wc.magic);
                continue;
            }
            sum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);
            if(wc.checksum != sum) {
                LELOGE("checksum = %d", wc.magic);
                continue;
            }
            LELOG("Get ssid[%s] passwd[%s]", wc.ssid, wc.wap2passwd);
            {
                PrivateCfg cfg;
                lelinkStorageReadPrivateCfg(&cfg);
                LELOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                        cfg.data.nwCfg.config.ssid,
                        cfg.data.nwCfg.config.psk, 
                        cfg.data.nwCfg.configStatus);
                strcpy(cfg.data.nwCfg.config.ssid, wc.ssid);
                strcpy(cfg.data.nwCfg.config.psk, wc.wap2passwd);
                cfg.data.nwCfg.configStatus = 1;
                ret = lelinkStorageWritePrivateCfg(&cfg);
                LELOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
            }
            break;
        } else {
            LELOGE("nwUDPRecvfrom ret = %d", ret);
        }
    }
out:
    if(ctx) {
        lelinkNwDelete(ctx);
    }
    halSoftApStop();
    return ret;
}

int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout)
{
    void *ctx;
    uint16_t port = 4911;
    int i, ret, count, delay = 1000; // ms
    char ipaddr[32] = "255.255.255.255";
    wificonfig_t wc = { WIFICONFIG_MAGIC, WIFICONFIG_VERSION, 0 };

    ctx  = lelinkNwNew(NULL, 0, 0, NULL);
    if(!ctx) {
        LELOGE("New link error");
        return -1;
    }
    count = timeout / delay + 1;
    strncpy((char *)wc.ssid, ssid, sizeof(wc.ssid));
    strncpy((char *)wc.wap2passwd, passwd, sizeof(wc.wap2passwd));
    wc.checksum = crc8((uint8_t *)&(wc.reserved), WIFICONFIG_CKSUM_LEN);
    for( i = 0; i < count; i++ ) {
        LELOG("Send wifi configure, [%s:%s][%d]...", ssid, passwd, delay);
        delayms(delay);
        ret = nwUDPSendto(ctx, ipaddr, port, (uint8_t *)&wc, sizeof(wc));
        if(ret <= 0 ) {
            LELOGE("nwUDPSendto ret = %d", ret);
        }
    }
    if(ctx) {
        lelinkNwDelete(ctx);
    }
    return 0;
}
