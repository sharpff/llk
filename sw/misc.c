#include "leconfig.h"
#include "misc.h"
#include "jsonv2.h"
#include "jsgen.h"
#include <stdarg.h>
#include "data.h"
#include "sengine.h"

static char miscBuf[MAX_BUF] = {0};

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
    int redirect = 0;
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
    setTerminalUTC((uint64_t *)&utc);

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

int getUartInfo(const char *json, int jsonLen, uartHandler_t* handler) {
    int ret = -1, num = 0, tmp, i = 0;
    char strBaud[96] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    char tmpStrMix[8] = {0};

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    if((ret = json_get_array_object(&jobj, JSON_NAME_UART_CONF, &num)) == WM_SUCCESS) {
        num = num < 0 ? 0 : num;
        for(i = 0; i < num; i++) {
            if((ret = json_array_get_composite_object(&jobj, i)) != WM_SUCCESS) {
                return -2;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_UART_ID, &tmp)) == WM_SUCCESS) {
                handler->id = tmp;
                LELOG("getUartInfo id[%d]", tmp);
            }
            if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_UART_BAUD, strBaud, sizeof(strBaud))) {
                return -3;
            }
        }
    }
    // TODO: to support multi-uart
    LELOG("getUartInfo baud[%s]", strBaud);
    sscanf(strBaud, "%u-%u%s", (unsigned int *)&handler->baud, (unsigned int *)&handler->dataBits, tmpStrMix);
    handler->parity = tmpStrMix[0];
    handler->stopBits = tmpStrMix[1] - 0x30;
    LELOG("getUartInfo baud[%d] [%d] [%c] [%d] [%s]", handler->baud, handler->dataBits, handler->parity, handler->stopBits, tmpStrMix);
    return 0;
}

int getGPIOInfo(const char *json, int jsonLen,  gpioHandler_t *table, int n) {
    jobj_t jobj;
    int i, num, ret, tmp, j = -1;
    jsontok_t jsonToken[NUM_TOKENS];
    LELOG("getGPIOInfo size[%d]", n);
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
                LELOG("getGPIOInfo json_get_val_int error");
                return -1;
            }

            table[j].id = tmp;
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_DIR, &tmp)) == WM_SUCCESS) {
                table[j].dir = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_MODE, &tmp)) == WM_SUCCESS) {
                table[j].mode = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_STATE, &tmp)) == WM_SUCCESS) {
                table[j].state = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_GPIO_TYPE, &tmp)) == WM_SUCCESS) {
                table[j].type = tmp;
            }
            LELOG("GPIO id = %d, dir = %d, mode = %d, state = %d, type = %d",
                    table[j].id, table[j].dir, table[j].mode, table[j].state, table[j].type);
            j++;
            json_release_composite_object(&jobj);
        }
        // json_release_array_object(jobj);
    }
    return j;
}

int getPipeInfo(const char *json, int jsonLen, char *name, int size) {
    jobj_t jobj;
    int ret = -1;
    jsontok_t jsonToken[NUM_TOKENS];

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if(WM_SUCCESS != ret) {
        return -1;
    }
    if(WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_PIPE_NAME, name, size)) {
        return -2;
    }
    return 0;
}

int getPWMInfo(const char *json, int jsonLen,  pwmHandler_t *table, int n) {
    jobj_t jobj;
    int i = -1, num, ret, tmp;
    jsontok_t jsonToken[NUM_TOKENS];
    if((ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen)) != WM_SUCCESS) {
        LELOG("getPWMInfo json_init");
        return -1;
    }
    if((ret = json_get_array_object(&jobj, JSON_NAME_PWM_CONF, &num))== WM_SUCCESS) {
        num = num > n ? n : num;
        LELOG("getPWMInfo num[%d]", num);
        for( i = 0; i < num; i++ ) {
            if((ret = json_array_get_composite_object(&jobj, i)) != WM_SUCCESS) {
                LELOG("getPWMInfo json_array_get_composite_object");
                return -1;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_ID, &tmp)) == WM_SUCCESS) {
                table[i].id = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_TYPE, &tmp)) == WM_SUCCESS) {
                table[i].type = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_CLOCK, &tmp)) == WM_SUCCESS) {
                table[i].clock = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_STATE, &tmp)) == WM_SUCCESS) {
                table[i].state = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_FREQUENCY, &tmp)) == WM_SUCCESS) {
                table[i].frequency = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_PWM_DUTY, &tmp)) == WM_SUCCESS) {
                table[i].duty = tmp;
            }
            LELOG("PWM id[%d], type[%d], clock[%d], fre[%d], duty[%d]",
                table[i].id, table[i].type, table[i].clock, table[i].frequency, table[i].duty);
            json_release_composite_object(&jobj);
        }
    }
    LELOG("getPWMInfo e[%d]", i);
    return i;
}

int getEINTInfo(const char *json, int jsonLen, eintHandler_t *table, int n) {
    jobj_t jobj;
    int i = -1, num, ret, tmp;
    jsontok_t jsonToken[NUM_TOKENS];
    if((ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen)) != WM_SUCCESS) {
        LELOG("getEINTInfo json_init");
        return -1;
    }
    if((ret = json_get_array_object(&jobj, JSON_NAME_EINT_CONF, &num))== WM_SUCCESS) {
        num = num > n ? n : num;
        LELOG("getEINTInfo num[%d]", num);
        for( i = 0; i < num; i++ ) {
            if((ret = json_array_get_composite_object(&jobj, i)) != WM_SUCCESS) {
                LELOG("getEINTInfo json_array_get_composite_object");
                return -1;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_ID, &tmp)) == WM_SUCCESS) {
                table[i].id = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_GID, &tmp)) == WM_SUCCESS) {
                table[i].gid = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_TYPE, &tmp)) == WM_SUCCESS) {
                table[i].type = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_MODE, &tmp)) == WM_SUCCESS) {
                table[i].mode = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_STATE, &tmp)) == WM_SUCCESS) {
                table[i].state = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_DEBOUNCE, &tmp)) == WM_SUCCESS) {
                table[i].debounce = tmp;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_EINT_TIMEOUT, &tmp)) == WM_SUCCESS) {
                table[i].timeout = tmp;
            }
            LELOG("EINT id[%d], gid[%d], mode[%d], debounce[%d], timeout[%d]",
                table[i].id, table[i].gid, table[i].mode, table[i].debounce, table[i].timeout);
            json_release_composite_object(&jobj);
        }
    }
    LELOG("getEINTInfo e[%d]", i);
    return i;
}

void convertStringToArray(char* str, commonManager_t *commonManager, uint8_t type) {
    char temp[8] = {0};
    int i, j = 0, count=0, len = strlen(str);
    memset(temp, 0, sizeof(temp));
    for(i=0; i<len+1; i++) {
        if(str[i] != '-' && str[i] != '\0') {
            temp[j++] = str[i];
        } else {
            if(type == 0) {
                commonManager->table[count++].id = atoi(temp);
            } else {
                commonManager->table[count++].mux = atoi(temp);
            }
            if(str[i]=='\0')
                return;
            memset(temp, 0, sizeof(temp));
            j = 0;
        }
    }
}

int getCommonInfo(const char *json, int jsonLen,  commonManager_t *commonManager, int n) {
    jobj_t jobj;
    int i, num, ret, tmp;
    char strTemp[256] = {0};
    jsontok_t jsonToken[NUM_TOKENS];
    if((ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen)) != WM_SUCCESS) {
        return -1;
    }
    if((ret = json_get_array_object(&jobj, JSON_NAME_COMMON_CONF, &num))== WM_SUCCESS) {
        num = num < 0 ? 0 : num;
        for(i = 0; i < num; i++) {
            if((ret = json_array_get_composite_object(&jobj, i)) != WM_SUCCESS) {
                LELOG("json_array_get_composite_object error %d", ret);
                return -2;
            }
            if((ret = json_get_val_int(&jobj, JSON_NAME_COMMON_NUM, &tmp)) == WM_SUCCESS) {
                commonManager->num = tmp;
            }
            if (WM_SUCCESS == json_get_val_str(&jobj, JSON_NAME_COMMON_ID, strTemp, sizeof(strTemp))) {
                convertStringToArray(strTemp, commonManager, 0);
                memset(strTemp, 0, sizeof(strTemp));
            }
            if (WM_SUCCESS == json_get_val_str(&jobj, JSON_NAME_COMMON_MUX, strTemp, sizeof(strTemp))) {
                convertStringToArray(strTemp, commonManager, 1);
            }
        }
    }
    for(i=0; i<commonManager->num; i++) {
        LELOG("Common index[%d] id[%d] mux[%d]", i, commonManager->table[i].id, commonManager->table[i].mux);
    }
    return i;
}

static int getJsonByToken(const char *json, int jsonLen, const char *key, char *obj, int objLen, const char *tokenStart, const char *tokenEnd, int fromBack) {
    char *start, *end, *oldEnd;
    int len = 0, max = 5;
    LELOG("getJsonByToken [%d][%s] key[%s] obj[%s] objLen[%d]", jsonLen, json, key, obj, objLen);
    start = (char *)strstr(json, key);
    if (NULL == start) {
        return -1;
    }

    start = (char *)strstr(start, tokenStart);
    if (NULL == start) {
        return -2;
    }

    if (fromBack) {
        oldEnd = end = start;
        do {
            oldEnd = end;
            end = (char *)strstr(end + 1, tokenEnd);
        } while (end && max--);
        if (oldEnd == start) {
            end = NULL;
        } else {
            end = oldEnd;
        }
    } else {
        end = (char *)strstr(start + 1, tokenEnd);
    }

    if (NULL == end) {
        return -3;
    }

    len = end - start + 1;
    if (objLen < len) {
        return -4;
    }
    LELOG("getJsonByToken len[%d] start[%d] end[%d]", len, start, end);

    // start[len] = 0;
    strncpy(obj, start, len);
    LELOG("getJsonByToken obj[%s]", obj);
    return len;    
}
int getJsonObject(const char *json, int jsonLen, const char *key, char *obj, int objLen) {
    char *tokenStart = "{", *tokenEnd = "}";
    return getJsonByToken(json, jsonLen, key, obj, objLen, tokenStart, tokenEnd, 0);
}

int getJsonArray(const char *json, int jsonLen, const char *key, char *obj, int objLen) {
    char *tokenStart = "[", *tokenEnd = "]";
    int ret = getJsonByToken(json, jsonLen, key, obj, objLen, tokenStart, tokenEnd, 1);
    if (2 >= ret) {
        return 0;
    }
    return ret;
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

int getCtrlData(const char *json, int jsonLen, const char *key, char *obj, int objLen) {
    char *start, *end, *temp;
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

    temp = start;

    while (1) {
        end = (char *)strstr(temp + 1, tokenEnd);
        if (NULL == end) {
            if(temp != start) {
                end = temp;
                break;
            } else {
                return -3;
            }
        }
        temp = end;
    }

    len = end - start;
    if (objLen < len) {
        return -4;
    }

    strncpy(obj, start, len);
    return len;
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
        LELOG("cloudMsgGetKey -e1");
        return CLOUD_MSG_KEY_NONE;
    }

    LELOG("cloudMsgGetKey [%d][%s]", jsonLen, json);

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        LELOG("cloudMsgGetKey -e2");
        return CLOUD_MSG_KEY_NONE;
    }

    if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_KEY, &key)) {
        LELOG("cloudMsgGetKey -e3");
        return CLOUD_MSG_KEY_NONE;
    }

    *retLen = getJsonObject(json, jsonLen, JSON_NAME_VAL, val, valLen);
    if (0 >= *retLen) {
        LELOG("cloudMsgGetKey -e4");
        return CLOUD_MSG_KEY_NONE;
    }

    return key;
}


static int ginLogSock;
static int ginDir;
static char ginIP[MAX_IPLEN];
static int ginPort;
// static int ginDir = 1;
// static char ginIP[MAX_IPLEN] = {"192.168.3.100"};
// static int ginPort = 1234;

void setLogDir(int dir) {
    ginDir = dir;
}

int getLogDir(void) {
    return ginDir;
}

void logToMaster(const char *log) {
    halNwUDPSendto(ginLogSock, ginIP, ginPort, (const uint8_t *)log, strlen(log));
}

int cloudMsgHandler(const char *data, int len) {

    int ret = WM_SUCCESS, dir = 0;
    CloudMsgKey key;
    char buf[MAX_BUF] = {0};
    memset(buf, 0, MAX_BUF);
    key = cloudMsgGetKey(data, len, buf, sizeof(buf), &ret);
    // SetLock();
    switch (key) {
        jsontok_t jsonToken[NUM_TOKENS];
        jobj_t jobj;
        case CLOUD_MSG_KEY_LOCK: {
            int locked = 0;
            ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)buf, ret);
            if (WM_SUCCESS != ret) {
                ret = LELINK_ERR_LOCK_UNLOCK;
                break;
            }

            if (WM_SUCCESS != (ret = json_get_val_int(&jobj, JSON_NAME_LOCK, &locked))) {
                ret = LELINK_ERR_LOCK_UNLOCK;
                break;
            }
            setLock(locked ? 1 : 0);
        }break;
        case CLOUD_MSG_KEY_DO_IA: {
            char name[MAX_RULE_NAME] = {0};
            ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)buf, ret);
            if (WM_SUCCESS != ret) {
                ret = LELINK_ERR_PARAM_INVALID;
                break;
            }

            if (WM_SUCCESS != (ret = json_get_val_str(&jobj, JSON_NAME_NAME, name, sizeof(name)))) {
                ret = LELINK_ERR_PARAM_INVALID;
                break;
            }
            if (0 >= sengineRemoveRules(name)) {
                ret = LELINK_ERR_IA_DELETE;
            }
        }break;
        case CLOUD_MSG_KEY_LOG2MASTER: {
            int broadcastEnable = 1;
            if (!ginLogSock) {
                ret = halNwNew(0, 0, &ginLogSock, &broadcastEnable);
                if (ret) {
                    LELOGW("halNwNew [%d]", ret);
                    ret = -1;
                    break;
                }        
            }

            key = cloudMsgGetKey(data, len, buf, sizeof(buf), &ret);
            LELOG("CLOUD_MSG_KEY_LOG2MASTER key[%d] [%d][%s]", key, ret, buf);
            if (CLOUD_MSG_KEY_LOG2MASTER != key) {
                LELOG("CLOUD_MSG_KEY_LOG2MASTER -e2");
                ret = -2;
                break;
            }
            ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)buf, ret);
            if (WM_SUCCESS != ret) {
                LELOG("CLOUD_MSG_KEY_LOG2MASTER -e3");
                ret = -3;
                break;
            }

            if (WM_SUCCESS != (ret = json_get_val_int(&jobj, JSON_NAME_LOG2MASTER, &dir))) {
                LELOG("CLOUD_MSG_KEY_LOG2MASTER -e4");
                ret = -4;
                break;
            }

            if (dir) {
                memset(ginIP, 0, sizeof(ginIP));
                if (WM_SUCCESS != (ret = json_get_val_str(&jobj, JSON_NAME_IP, ginIP, sizeof(ginIP)))) {
                    LELOG("CLOUD_MSG_KEY_LOG2MASTER -e5");
                    break;
                }
                if (WM_SUCCESS != (ret = json_get_val_int(&jobj, JSON_NAME_PORT, &ginPort))) {
                    LELOG("CLOUD_MSG_KEY_LOG2MASTER -e6");
                    break;
                }
            }

            setLogDir(dir);
            LELOG("CLOUD_MSG_KEY_LOG2MASTER [%d] -e", dir);
        }break;
        default:
        break;
    }
    return ret == WM_SUCCESS ? 1 : ret;
}

int printOut(const char *fmt, ...) {
    va_list args;
    memset(miscBuf, 0, MAX_BUF);
    va_start(args, fmt);
    vsnprintf(miscBuf, sizeof(miscBuf), fmt, args);
    va_end(args);
    if (getLogDir()) {
        logToMaster(miscBuf);
        halPrint(miscBuf);
    } else {
        halPrint(miscBuf);
    }
    return 0;
}