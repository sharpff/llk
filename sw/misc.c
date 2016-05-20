#include "leconfig.h"
#include "misc.h"
#include "jsonv2.h"
#include "jsgen.h"
#include <stdarg.h>

#define NUM_TOKENS 256

// 
#define JSON_NAME_REDIRECT "redirect"
#define JSON_NAME_IP "IP"
#define JSON_NAME_PORT "port"
#define JSON_NAME_UTC "utc"
#define JSON_NAME_WHATTYPE "whatCvtType"
#define JSON_NAME_BAUD "baud"
#define JSON_NAME_STATUS "status"
#define JSON_NAME_UUID "uuid"
#define JSON_NAME_URL "url"
#define JSON_NAME_TYPE "type"
#define JSON_NAME_GPIO_CONF         "conf"
#define JSON_NAME_GPIO_ID           "id"
#define JSON_NAME_GPIO_DIR          "dir"
#define JSON_NAME_GPIO_MODE         "mode"
#define JSON_NAME_GPIO_BLINK        "blink"
#define JSON_NAME_GPIO_STATE        "state"
#define JSON_NAME_GPIO_TYPE         "type"
#define JSON_NAME_GPIO_TIME_SHORT   "shortTime"
#define JSON_NAME_GPIO_TIME_LONG    "longTime"
#define JSON_NAME_KEY               "key"
#define JSON_NAME_VAL               "val"
#define JSON_NAME_NAME              "name"
#define JSON_NAME_LOCK              "lock"

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port) {
    int ret = -1;
    int dir = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return 0;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_REDIRECT, &dir)) {
        return 0;
    }

    if (!dir) {
        return 0;
    }

    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_IP, ip, MAX_IPLEN)) {
        return 0;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_PORT, (int *)port)) {
        return 0;
    }

    return 1;
}

int syncUTC(const char *json, int jsonLen) {
    int ret = 0;
    int64_t utc = 0;
    int32_t redirect = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    // json_string_t jretobj = {0};

    // LELOG("syncUTC [0x%x, %d], [0x%x, %d], [0x%x, %d]", jsonToken, sizeof(jsonToken), &jobj, sizeof(jobj), &jretobj, sizeof(jretobj));
    // LELOG("jobj 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", &(jobj.parser), jobj.js, jobj.tokens, jobj.cur, &(jobj.num_tokens));
    memset(jsonToken, 0, sizeof(jsonToken));
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    // json_str_init(&jretobj, json, jsonLen);
    // json_start_object(&jretobj);
    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_UTC, &redirect)) {
        return -2;
    }

    if (WM_SUCCESS != json_get_val_int64(&jobj, JSON_NAME_UTC, &utc)) {
        return -3;
    }
    setTerminalUTC(&utc);

    return 0;
    // json_close_object(&jretobj);

    // int ret = -1;
    // uint32_t utc = 0;
    // jsontok_t jsonToken[NUM_TOKENS];
    // jobj_t jobj;

    // ret = json_init(&jobj, jsonToken, NUM_TOKENS, json, jsonLen);
    // if (WM_SUCCESS != ret) {
    //     return;
    // }

    // if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_UTC, &utc)) {
    //     return;
    // }

    // setTerminalUTC(utc);
}

int genCompositeJson(const char *json, int jsonLen, int count, ...) {
    va_list ap;
    char *ptrName = NULL;
    char *ptrVal = NULL;
    int i = 0;
    int ret = -1;
    // int dir = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    json_string_t jretobj = {0};

    // LELOG("genCompositeJson 0x%x, 0x%x, 0x%x", jsonToken, &jobj, &jretobj);
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return 0;
    }

    json_str_init(&jretobj, (char *)json, jsonLen);
    json_start_object(&jretobj);

    va_start(ap, count);
    for(i = 0; i < count; i++)
    {
        ptrName = va_arg(ap, char *);
        ptrVal = va_arg(ap, char *);
        json_set_val_strobj(&jretobj, ptrName, ptrVal, strlen(ptrVal));
        LELOG("[%s]->[%s]", ptrName, ptrVal);
    }
    va_end(ap);

    json_close_object(&jretobj);

    return 0;
}

int getJsonUTC(char *json, int jsonLen) {
    int64_t utc = 0;
    getTerminalUTC(&utc);
    return sprintf(json, "{\"utc\":%lld}", utc);
}

int getJsonUTC32(char *json, int jsonLen/*, const char *rmtJson, int rmtJsonLen*/) {
    int64_t utc = 0;
    uint32_t utcH = 0, utcL = 0;
    // int ret = 0;
    // jsontok_t jsonToken[NUM_TOKENS];
    // jobj_t jobj;

    // if (NULL == rmtJson || 0 >= rmtJsonLen) {
        getTerminalUTC(&utc);
    // } else {
    //     ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)rmtJson, rmtJsonLen);
    //     if (WM_SUCCESS != ret) {
    //         return -1;
    //     }

    //     if (WM_SUCCESS != json_get_val_int64(&jobj, JSON_NAME_UTC, &utc)) {
    //         return -2;
    //     }
    // }

    utcH = (uint32_t)(utc >> 32);
    utcL = (uint32_t)utc;
    return sprintf(json, "{\"utcH\":%d,\"utcL\":%d}", utcH, utcL);
}

int getWhatCvtType(const char *json, int jsonLen) {
    int ret = -1, whatCvtType = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_WHATTYPE, &whatCvtType)) {
        return -2;
    }

    return whatCvtType;
}

int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl) {
    int ret = -1;
    char strBaud[96] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_BAUD, strBaud, sizeof(strBaud))) {
        return -2;
    }

    // TODO: adption
    sscanf(strBaud, "%u-%u%c%u", baud, dataBits, parity, stopBits);

    return 0;
}

int getGPIOInfo(const char *json, int jsonLen,  gpioHand_t *table, int n)
{
    jobj_t jobj;
    int i, k, num, ret, tmp, j = -1;
    jsontok_t jsonToken[NUM_TOKENS];

    if((ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen)) != WM_SUCCESS) {
        return -1;
    }
    if((ret = json_get_array_object(&jobj, JSON_NAME_GPIO_CONF, &num))== WM_SUCCESS) {
        num = num > n ? n : num;
        for( i = 0, j = 0; i < num; i++ ) {
            if((ret = json_array_get_composite_object(&jobj, i)) != WM_SUCCESS) {
                return -1;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_ID, &tmp)) != WM_SUCCESS) {
                return -1;
            }
            for( k = 0; k < i; k++ ) {
                if(table[k].id == tmp) {
                    tmp = -1;
                    break;
                }
            }
            if(tmp <= 0 || tmp > n) {
                continue;
            }
            table[j].id = tmp;
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_DIR, &tmp)) == WM_SUCCESS) {
                table[j].dir = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_MODE, &tmp)) == WM_SUCCESS) {
                table[j].mode = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_BLINK, &tmp)) == WM_SUCCESS) {
                table[j].blink = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_STATE, &tmp)) == WM_SUCCESS) {
                table[j].state = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_TYPE, &tmp)) == WM_SUCCESS) {
                table[j].type = tmp;
            }
            if((table[j].dir == GPIO_DIR_INPUT && table[j].type == GPIO_TYPE_INPUT_RESET) || 
                    (table[j].dir == GPIO_DIR_OUTPUT && table[j].type == GPIO_TYPE_OUTPUT_RESET)) {
                if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_TIME_SHORT, &tmp)) != WM_SUCCESS || tmp < 1) {
                    LELOGE("GPIO wrang value: %s = %d",  JSON_NAME_GPIO_TIME_SHORT, (ret == WM_SUCCESS ? tmp : ret));
                    continue;
                }
                table[j].shortTime = tmp;
                if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_TIME_LONG, &tmp)) != WM_SUCCESS || tmp <= table[j].shortTime) {
                    LELOGE("GPIO wrang value: %s = %d",  JSON_NAME_GPIO_TIME_LONG, (ret == WM_SUCCESS ? tmp : ret));
                    continue;
                }
                table[j].longTime = tmp;
            }
            /*LELOGE("IO id = %d, num = %d, dir = %d, mode = %d, state = %d, type = %d, blink = %d", */
                    /*table[j].id, table[j].num, table[j].dir, table[j].mode, table[j].state, table[j].type, table[j].blink);*/
            j++;
            json_release_composite_object(&jobj);
        }
        // json_release_array_object(jobj);
    }
    return j;
}

int getJsonObject(const char *json, int jsonLen, const char *key, char *obj, int objLen) {
    char *start, *end;
    char *tokenStart = "{", *tokenEnd = "}";
    int len = 0;
    start = (char *)strstr(json, key);
    if (NULL == start) {
        return -1;
    }

    start = (char *)strstr(start, tokenStart);
    if (NULL == start) {
        return -2;
    }

    end = (char *)strstr(start + 1, tokenEnd);
    if (NULL == end) {
        return -3;
    }

    len = end - start + 1;
    if (objLen < len) {
        return -4;
    }

    // start[len] = 0;
    strncpy(obj, start, len);
    return len;
}

int genS2Json(const char *status, int statusLen, const char *rmtJson, int rmtJsonLen, char *result, int resultLen) {
    int ret = 0, tmpLen;
    char utc[64] = {0};
    // char nowStatus[1024] = {0};
    // char *pResult = result;
    const char *key1 = "\"status\"";
    // const char *key2 = "\"uuid\"";
    // jsontok_t jsonToken[NUM_TOKENS];
    // jobj_t jobj;

    ret = getJsonUTC32(utc, sizeof(utc)/*, rmtJson, rmtJsonLen*/);
    if (0 >= ret) {
        return ret;
    }

    tmpLen = sprintf(result, "{%s:", key1);
    ret = getJsonObject(status, statusLen, key1, result + tmpLen, resultLen - tmpLen);
    if (0 >= ret) {
        return ret;
    }
    tmpLen += ret;

    utc[0] = ',';
    ret = sprintf(result + tmpLen, "%s", utc);
    tmpLen += ret;

    strcpy(result + tmpLen - 1, ",\"uuid\":\""); tmpLen = strlen(result);
    if (NULL == rmtJson || 0 >= rmtJsonLen) {
        // uuid
        getTerminalUUID((uint8_t *)result + tmpLen, MAX_UUID); tmpLen = strlen(result);
 
    } else {
        ret = getUUIDFromJson(rmtJson, rmtJsonLen, result + tmpLen, resultLen - tmpLen);
        if (0 >= ret) {
            return ret;
        }
        tmpLen += ret;
    }
    result[tmpLen] = '"'; 
    tmpLen += 1;
    result[tmpLen] = '}'; 
    tmpLen += 1;       
    return tmpLen;
}

// TODO: timer start for heart beat, hal need to support
void startHeartBeat(void) {

}

int getUUIDFromJson(const char *json, int jsonLen, char *uuid, int uuidLen) {
    int ret = 0;
    // char strBaud[96] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    if (NULL == json || 0 >= jsonLen) {
        return -1;
    }

    LELOG("getUUIDFromJson [%d][%s]", jsonLen, json);

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -2;
    }

    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_UUID, uuid, uuidLen)) {
        return -3;
    }

    return strlen(uuid);
}

int getJsonOTAType(const char *json, int jsonLen, char *url, int urlLen) {
    int ret = 0, type = -1;
    // char strBaud[96] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    if (NULL == json || 0 >= jsonLen) {
        return -1;
    }

    LELOG("getJsonOTAType [%d][%s]", jsonLen, json);

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -2;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_TYPE, &type)) {
        return -3;
    }


    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_URL, url, urlLen)) {
        return -4;
    }

    return type;
}

CloudMsgKey cloudMsgGetKey(const char *json, int jsonLen, char *val, int valLen, int *retLen) {
    int ret = 0, key = 0;
    // char strBaud[96] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    if (NULL == json || 0 >= jsonLen) {
        return CLOUD_MSG_KEY_NONE;
    }

    LELOG("cloudMsgGetKey [%d][%s]", jsonLen, json);

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return CLOUD_MSG_KEY_NONE;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_KEY, &key)) {
        return CLOUD_MSG_KEY_NONE;
    }

    *retLen = getJsonObject(json, jsonLen, JSON_NAME_VAL, val, valLen);
    if (0 >= *retLen) {
        return CLOUD_MSG_KEY_NONE;
    }

    return key;
}


int cloudMsgHandler(const char *data, int len) {

    int ret = 0;
    char buf[MAX_BUF] = {0};
    CloudMsgKey key = cloudMsgGetKey(data, len, buf, sizeof(buf), &ret);
    // SetLock();
    switch (key) {
        jsontok_t jsonToken[NUM_TOKENS];
        jobj_t jobj;
        case CLOUD_MSG_KEY_LOCK: {
            char name[MAX_RULE_NAME] = {0};
            int locked = 0;
            ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)buf, ret);
            if (WM_SUCCESS != ret) {
                break;
            }

            if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_LOCK, &locked)) {
                break;
            }
            setLock(locked ? 1 : 0);
        }break;
        case CLOUD_MSG_KEY_DO_IA: {
            char name[MAX_RULE_NAME] = {0};
            ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)buf, ret);
            if (WM_SUCCESS != ret) {
                break;
            }

            if (WM_SUCCESS != (ret = json_get_val_str(&jobj, JSON_NAME_NAME, name, sizeof(name)))) {
                break;
            }

            // TODO: 
            sengineRemoveRules(name);
        }break;
        case CLOUD_MSG_KEY_DO_SHARE: {

        }break;
    }
    return ret;
}
