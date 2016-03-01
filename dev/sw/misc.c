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
        LELOG("[%s]->[%s]\r\n", ptrName, ptrVal);
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

int getJsonUTC32(char *json, int jsonLen) {
    int64_t utc = 0;
    uint32_t utcH = 0, utcL = 0;
    getTerminalUTC(&utc);
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

int genS2Json(const char *status, int statusLen, char *result, int resultLen) {
    int ret = 0, tmpLen;
    char utc[64] = {0};
    // char nowStatus[1024] = {0};
    // char *pResult = result;
    const char *key = "\"status\"";
    // jsontok_t jsonToken[NUM_TOKENS];
    // jobj_t jobj;

    ret = getJsonUTC32(utc, sizeof(utc));
    if (0 >= ret) {
        return ret;
    }

    tmpLen = sprintf(result, "{%s:", key);
    ret = getJsonObject(status, statusLen, key, result + tmpLen, resultLen - tmpLen);
    if (0 >= ret) {
        return ret;
    }
    tmpLen += ret;

    utc[0] = ',';
    ret = sprintf(result + tmpLen, "%s", utc);
    tmpLen += ret;

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

    LELOG("getUUIDFromJson [%s]\r\n", json);

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)json, jsonLen);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_UUID, uuid, uuidLen)) {
        return -2;
    }

    return strlen(uuid);
}