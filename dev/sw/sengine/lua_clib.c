
#define lua_clib_c
#include <stdlib.h>

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "llimits.h"
#include "lmem.h"
#include "lstate.h"
#include "lzio.h"
#include "lua_clib.h"
#include "lauxlib.h"


/* Define GETINT to get a integer value */
#define GETINT(L, n)   (luaL_checkinteger((L), (n)))

#define MONADIC(name, op)							\
int bit ## name(lua_State *L) {						\
	lua_pushinteger(L, op GETINT(L, 1));				\
	return 1;										\
}

#define VARIADIC(name, op)                      \
int bit ## name(lua_State *L) {					\
	int n = lua_gettop(L), i;                   \
	lua_Integer w = GETINT(L, 1);                \
	for (i = 2; i <= n; i++)                    \
		w op GETINT(L, i);						\
	lua_pushinteger(L, w);                      \
	return 1;                                   \
}

#define LOGICAL_SHIFT(name, op)						\
int bit ## name(lua_State *L) {						\
	lua_pushinteger(L, (size_t)GETINT(L, 1) op		\
	(unsigned)luaL_checknumber(L, 2));				\
	return 1;										\
}

//int bitshift(lua_State *L);
//int bitor(lua_State *L);
//int bitand(lua_State *L);

VARIADIC(and, &= )
VARIADIC(or, |= )
VARIADIC(xor, ^= )
MONADIC(nor, ~ )
LOGICAL_SHIFT(shift, >> )
LOGICAL_SHIFT(shiftL, << )


/*int bitshift(lua_State *L)
{
	return 0;
}
int bitor(lua_State *L)
{
	return 0;
}
int bitand(lua_State *L)
{
	return 0;
}*/

#if 0

int csum(lua_State *L)
{
    //int i = 0;
    char strsum[3] = { 0 };
    // unsigned char sum = 0;

    const char *str = (const char *)lua_tostring(L, 1);
    if (NULL == str)
    {
        lua_pushstring(L, strsum);
        return 1;
    }
    unsigned char length = lua_tointeger(L, 2);
    unsigned char *data = (unsigned char *)malloc(length);
    memcpy(data, str, length);
    unsigned char *tmp = data;
    //int type = lua_tointeger(L, 3);
    //int type2 = lua_tointeger(L, 4);
    //int type3 = lua_tointeger(L, 5);

    unsigned char i, j;
    unsigned short crc;
    crc = 0xffff;
    for (i = 0; i<length; i++)
    {
        crc ^= *tmp++;
        for (j = 0; j<8; j++)
        {
            if (crc & 0x01)
            {
                crc = (crc >> 1) ^ 0x8408;
            }
            else
            {
                crc >>= 0x01;
            }
        }
    }
    crc = ~crc;
    free(data);

    //memcpy(strsum, &crc, sizeof(crc));
    strsum[0] = (unsigned char)(crc >> 8);
    strsum[1] = (unsigned char)crc;
    lua_pushstring(L, strsum);
    LELOG("xxxxxxxxxxxxx check sum is [%02x, %02x]\r\n", strsum[0], strsum[1]);
    return 1;
}
#endif

#if defined(__MRVL_MW300__)
inline float floorf(float x) {
    return (float)(x < 0.f ? (((int)x) - 1) : ((int)x));
}
inline double floor(double x) {
    return (double)(x < 0.f ? (((int)x) - 1) : ((int)x));
}
inline void _exit(int status) {
    LELOG("_exit\r\n");
    while (1);
}
typedef int FILEHANDLE;
inline int _close(FILEHANDLE fh) {
    LELOG("_close\r\n");
    return 0;
}
inline int _write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) {
    LELOG("_write\r\n");
    return 0;
}
inline int _read(FILEHANDLE fh, unsigned char*buf, unsigned len, int mode) {
    LELOG("_read\r\n");
    return 0;
}
inline int _lseek(FILEHANDLE fh, long pos) {
    LELOG("_lseek\r\n");
    return 0;
}
inline long _fstat(FILEHANDLE fh) {
    LELOG("_fstat\r\n");
    return 0;
}
inline int _isatty(FILEHANDLE fh) {
    LELOG("_isatty\r\n");
    return 0;
}
inline int _kill(FILEHANDLE fh) {
    LELOG("_kill\r\n");
    return 0;
}
inline int _getpid(FILEHANDLE fh) {
    LELOG("_getpid\r\n");
    return 0;
}
inline int _sbrk(void) {
    LELOG("_sbrk\r\n");
    return 0;
}
#elif defined (LINUX)
#else
#error ("no adpation...")

#endif


