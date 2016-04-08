#include "lprefix.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF 1024
#define MAX_STATUS 64
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define WM_SUCCESS 0
#define wmprintf printf 
#define os_mem_alloc calloc
#define os_mem_free free
typedef unsigned int		uint32_t;
typedef unsigned short  	uint16_t;
typedef unsigned char	    uint8_t;

#if defined (WIN32)
#define ALIGNED
#else
#define ALIGNED __attribute__((packed))
#endif
#include "sengine.h"

char filename[128] = { 0 };
uint32_t filesize = 0;

#define FILE_JSON "json"

static char g_json[2048];
static uint8_t g_buf[MAX_BUF] = { 0 };
static uint16_t g_bufLen = 0;
static uint8_t g_bsta[MAX_BUF] = { 0 };
static int g_dir;
//#define LUA_PARAM 4
//#define LUA_FUNC "devQuery"

//typedef void (*LF_IMPL)(lua_State *L, int param_num);




/* hex string to bytes*/
static int hexStr2bytes(const char *hexStr, unsigned char *buf, int bufLen) {
	int i;
	int len;

	if (NULL == hexStr) {
		len = 0;
	}
	else {
		len = (int)strlen(hexStr) / 2;

		if (bufLen < len)
			len = bufLen;
	}

	    // sscanf杩囦簬鑰楄垂鍐呭瓨锛岄噰鐢ㄥ涓嬫柟寮忚浆鎹?

	memset(buf, 0, bufLen);

	for (i = 0; i < len; i++) {
		char ch1, ch2;
		int val;

		ch1 = hexStr[i * 2];
		ch2 = hexStr[i * 2 + 1];
		if (ch1 >= '0' && ch1 <= '9')
			val = (ch1 - '0') * 16;
		else if (ch1 >= 'a' && ch1 <= 'f')
			val = ((ch1 - 'a') + 10) * 16;
		else if (ch1 >= 'A' && ch1 <= 'F')
			val = ((ch1 - 'A') + 10) * 16;
		else
			return -1;

		if (ch2 >= '0' && ch2 <= '9')
			val += ch2 - '0';
		else if (ch2 >= 'a' && ch2 <= 'f')
			val += (ch2 - 'a') + 10;
		else if (ch2 >= 'A' && ch2 <= 'F')
			val += (ch2 - 'A') + 10;
		else
			return -1;

		buf[i] = val & 0xff;
	}

	return len;
}

//#include <Windows.h>
int main(int argc, char *argv[]) {

	ginScriptCfg = (ScriptCfg *)malloc(sizeof(ScriptCfg));
	memset(ginScriptCfg, 0, sizeof(ScriptCfg));
	//// GETSTATUS
	//if (strstr(argv[3], GETSTATUS)) {

	    //}
	    //// STD2PRI
	    //else if (strstr(argv[3], FILE_JSON))
	    //{
	    //    g_dir = 1;
	    //    FILE *fp1 = fopen(argv[3], "rb");
	    //    if (fp1)
	    //    {
	    //        fread(g_json, sizeof(g_json), 1, fp1);
	    //        fclose(fp1);
	    //    }

	        //    if (5 == argc)
	        //    {
	        //        hexStr2bytes(argv[4], g_buf, sizeof(g_buf));
	        //        //printf("ser data is [%s]\r\n", g_buf);
	        //    }
	        //}
	        //// PRI2STD
	        //else
	        //{
	        //    g_dir = 0;
	        //    size_t i = 0;
	        //    g_bufLen = hexStr2bytes(argv[3], g_buf, sizeof(g_buf));
	        //    //strcpy(g_buf, argv[3]);
	        //    //memcpy(g_buf, argv[3], strlen(argv[3]));
	        //    //printf("PRI2STD:\r\n");
	        //    //for (i = 0; i < strlen(argv[3]); i++)
	        //    //{
	        //    //    //g_buf[i] = argv[3][i];
	        //    //    printf("%02x ", g_buf[i]);
	        //    //}
	        //    //printf("\r\n");
	        //}
	char func_name[128] = { 0 };
	uint8_t *inputParam = NULL;
	uint8_t *inputParam2 = NULL;
	int inputParamLen = 0;
	int inputParamLen2 = 0;
	uint8_t buf[MAX_BUF] = { 0 };
	if (argc < 3) {
ERROR_PARAM:
		printf("exec [lua] [func name] [param/json | buf if translate for PRI2STD] [resp cache if jsonfile | optional]\r\n");
		return 0;
	}
	strcpy(filename, argv[1]);
	FILE *fp = fopen(filename, "rb");
	strcpy(func_name, argv[2]);

	    // only for luait.py. in case of std2pri.json
	if (argv[3] && strstr(argv[3], FILE_JSON)) {
		FILE *fp1 = fopen(argv[3], "rb");
		if (fp1) {
			int filesize1 = 0;
			fseek(fp1, 0, SEEK_END);
			filesize1 = ftell(fp1);
			fseek(fp1, 0 - (filesize1), SEEK_END);

			fread(g_buf, sizeof(g_buf), 1, fp1);
			fclose(fp1);

			inputParam = g_buf;
			inputParamLen = filesize1;
		}
	}
	else if (argv[2] && strstr(argv[2], S1_GET_VALIDKIND)) {
		if (!argv[3]) {
			goto ERROR_PARAM;
		}
		g_bufLen = hexStr2bytes(argv[3], g_buf, sizeof(g_buf));
		inputParam = g_buf;
		inputParamLen = g_bufLen;
	}
	else if (argv[2] && strstr(argv[2], S1_STD2PRI)) {
		if (!argv[3]) {
			goto ERROR_PARAM;
		}
		inputParam = argv[3];
		inputParamLen = strlen(inputParam);
	}
	else if (argv[2] && strstr(argv[2], S1_GET_CVTTYPE)) {
		inputParam = NULL;
		inputParamLen = 0;
	}
	else if (argv[2] && strstr(argv[2], S1_PRI2STD)) {
		if (!argv[3]) {
			goto ERROR_PARAM;
		}
		g_bufLen = hexStr2bytes(argv[3], g_buf, sizeof(g_buf));
		inputParam = g_buf;
		inputParamLen = g_bufLen;
	}
	else if (argv[2] && strstr(argv[2], S1_GET_QUERIES)) {
		inputParam = NULL;
		inputParamLen = 0;
	}
	else if (argv[2] && strstr(argv[2], S1_GET_VER)) {
		inputParam = NULL;
		inputParamLen = 0;
	}
	//else if (argv[2] && strstr(argv[2], S2_GET_RULEINFO)) {
	//    inputParam = NULL;
	//    inputParamLen = 0;
	//}
	else if (argv[2] && strstr(argv[2], S2_IS_VALID) || 
	    argv[2] && strstr(argv[2], S2_IS_VALID_EXT)) {
		if (!argv[3]) {
			goto ERROR_PARAM;
		}
		inputParam = argv[3];
		inputParamLen = strlen(inputParam);
	}
	else if (argv[2] && strstr(argv[2], S2_GET_RULETYPE)) {
		inputParam = NULL;
		inputParamLen = 0;        
	}
	else if (argv[2] && strstr(argv[2], S2_GET_BERESERVED)) {
		inputParam = NULL;
		inputParamLen = 0;        
	}
	else if (argv[2] && 0 == strcmp(argv[2], S2_GET_ISOK_EXT)) {
		if (!argv[3] || !argv[4]) {
			goto ERROR_PARAM;
		}
		inputParam = argv[3];
		inputParam2 = argv[4];
		inputParamLen = strlen(inputParam);
		inputParamLen2 = strlen(inputParam2);
		strcpy(buf, inputParam);
		buf[inputParamLen] = 0;
		strcpy(buf + inputParamLen + 1, inputParam2);
		inputParam = buf;
		inputParamLen = inputParamLen + 1 + inputParamLen2;
	}
	else if (argv[2] && 0 == strcmp(argv[2], S2_GET_ISOK)) {
		if (!argv[3]) {
			goto ERROR_PARAM;
		}
		inputParam = argv[3];
		inputParam2 = argv[4];
		inputParamLen = strlen(inputParam);
		inputParamLen2 = strlen(inputParam2);
		strcpy(buf, inputParam);
		buf[inputParamLen] = 0;
		strcpy(buf + inputParamLen + 1, inputParam2);
		inputParam = buf;
		inputParamLen = inputParamLen + 1 + inputParamLen2;
	}
	else if (argv[2] && strstr(argv[2], S2_GET_BECMD)) {
		inputParam = NULL;
		inputParamLen = 0;      
	}
	else {
		goto ERROR_PARAM;
	}
	if (fp) {
		int ret = 0;
		char json[1024] = { 0 };
		fseek(fp, 0, SEEK_END);
		ginScriptCfg->data.size = filesize = ftell(fp);
		fseek(fp, 0 - (filesize), SEEK_END);
		memset(ginScriptCfg->data.script, 0, ginScriptCfg->data.size);
		fread(ginScriptCfg->data.script, ginScriptCfg->data.size, 1, fp);
#ifndef TEST_ONLY
		ret = sengineCall((const char *)ginScriptCfg->data.script,
			ginScriptCfg->data.size,
			func_name,
			inputParam,
			inputParamLen,
			(uint8_t *)json,
			sizeof(json));

		fclose(fp);
#endif
	}
#ifdef TEST_ONLY
	{
		extern int ss2apiGetStatusTest(lua_State *L);
		extern int s2apiGetLatestStatus(lua_State *L);
		extern void testOnlySetIACache();

		int s2apiSetCurrStatus(lua_State *L);
		lua_State *L = luaL_newstate();
		int param = 2;
		int ret = 1;

		testOnlySetIACache();

		lua_register(L, "ss2apiGetStatusTest", ss2apiGetStatusTest);
		lua_register(L, "s2apiGetLatestStatus", s2apiGetLatestStatus);
		lua_register(L, "s2apiSetCurrStatus", s2apiSetCurrStatus);
		luaL_openlibs(L);
		if (luaL_loadbuffer(L, ginScriptCfg->data.script, ginScriptCfg->data.size, "lelink") || lua_pcall(L, 0, 0, 0)) {
			lua_pop(L, 1);
			ret = -1;
			printf("[lua engine] lua code syntax error\r\n");
		}
		else {
			int selfLen = 0, rmtLen = 0;
			char p[1024] = { 0 };

			lua_getglobal(L, S2_GET_ISOK_EXT);

			strcpy(p, "{\"status\":{\"qca9531\":1,\"pwr\":1},\"uuid\":\"10000100101000010007F0B429000012\"}");
			selfLen = strlen(p); p[selfLen] = 0;
			lua_pushlstring(L, (char *)p, selfLen);

			strcpy(p + selfLen + 1, "{\"status\":{\"idx2\":1,\"idx3\":2},\"uuid\":\"10000100101000010007FFFFFFFFFFFF\"}");
			rmtLen = strlen(p + selfLen + 1); p[selfLen + 1 + rmtLen] = 0;
			lua_pushlstring(L, (char *)p + selfLen + 1, rmtLen);

			if (lua_pcall(L, param, ret, 0)) {
				const char *err = lua_tostring(L, -1);
				printf("[%s]\r\n", err);
				lua_pop(L, 1);
			}
			else {
				lua_pop(L, ret);
			}
		}

	}


#endif
	return 0;
}

int ss2apiGetStatusTest(lua_State *L) {
	int i = 0;

	    /* create table. */
	lua_newtable(L);

	    /* push (key, value). */
	char value1[16] = { 0 };
	for (i = 0; i < 3; i++) {
		sprintf(value1, "1st value %d", i + 1);
		lua_pushnumber(L, i + 1);    //key  
		lua_pushstring(L, value1);  //value  
		lua_settable(L, -3);       //push key,value  
	}

	    ///* create table. */
	    //lua_newtable(L);

	        ///* push (key, value). */
	        //char value2[16] = { 0 };
	        //for (i = 0; i < 2; i++) {
	        //    sprintf(value2, "2nd value %d", i + 1);
	        //    lua_pushnumber(L, i + 1);    //key  
	        //    lua_pushstring(L, value2);  //value  
	        //    lua_settable(L, -3);       //push key,value  
	        //}
	return 1;
}
