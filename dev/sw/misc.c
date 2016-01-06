#include "misc.h"
#include "jsonv2.h"
#include "jsgen.h"
#include <stdarg.h>

#define NUM_TOKENS 256

// 
#define JSON_NAME_REDIRECT "dir"
#define JSON_NAME_IP "IP"
#define JSON_NAME_PORT "port"

int isNeedToRedirect(char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port) {
    int ret = -1;
    int dir = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    json_parser_t jobj;

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

void syncUTC(const char *json, int jsonLen) {

}

int genCompositeJson(char *json, int jsonLen, int count, ...) {
    va_list ap;
    char *ptrName = NULL;
    char *ptrVal = NULL;
    int i = 0;
    int ret = -1;
    // int dir = 0;
    jsontok_t jsonToken[NUM_TOKENS];
    json_parser_t jobj;
    json_string_t jretobj = {0};

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

int halGetJsonUTC(char *json, int jsonLen) {
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