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

int isNeedToRedirect(char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port) {
    int ret = -1;
    int dir = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, json, jsonLen);
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

int syncUTC(char *json, int jsonLen) {
    int ret = 0;
    int64_t utc = 0;
    int32_t redirect = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    jobj_t jobj;
    // json_string_t jretobj = {0};

    // LELOG("syncUTC [0x%x, %d], [0x%x, %d], [0x%x, %d]", jsonToken, sizeof(jsonToken), &jobj, sizeof(jobj), &jretobj, sizeof(jretobj));
    // LELOG("jobj 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", &(jobj.parser), jobj.js, jobj.tokens, jobj.cur, &(jobj.num_tokens));
    memset(jsonToken, 0, sizeof(jsonToken));
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, json, jsonLen);
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

int genCompositeJson(char *json, int jsonLen, int count, ...) {
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
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, json, jsonLen);
    if (WM_SUCCESS != ret) {
        return 0;
    }

    json_str_init(&jretobj, json, jsonLen);
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
	// TODO: 
    const char *tmp = "{\"utc\":123412341234}";
    // int ret = 0;

    if (strlen(tmp) >= jsonLen) {
        return 0;
    }

    memcpy(json, tmp, strlen(tmp));
    return strlen(tmp);
}

// TODO: timer start for heart beat, hal need to support
void startHeartBeat(void) {

}

// int getUUIDFromJson(char *json, int jsonLen, char uuid[MAX_UUID]) {
//     return 0;
// }