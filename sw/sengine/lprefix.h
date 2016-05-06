/*
** $Id: lprefix.h,v 1.2 2014/12/29 16:54:13 roberto Exp $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#ifndef lprefix_h
#define lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#if !defined(LUA_USE_C89)	/* { */

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE           600
#elif _XOPEN_SOURCE == 0
#undef _XOPEN_SOURCE  /* use -D_XOPEN_SOURCE=0 to undefine it */
#endif

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#if !defined(LUA_32BITS) && !defined(_FILE_OFFSET_BITS)
#define _LARGEFILE_SOURCE       1
#define _FILE_OFFSET_BITS       64
#endif

#endif				/* } */


/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#endif

#endif			/* } */


#if defined(__MRVL_MW300__)
#include <wmerrno.h>
#include <stdarg.h>
#include <wmstdio.h>
#include <stdlib.h>
#include <string.h>
#include <wmtime.h>
#include "FreeRTOS.h"
#define malloc(s) pvPortMalloc(s)
#define realloc(p, s) pvPortReAlloc(p, s)
#define free(p) vPortFree(p)
#define memset hal_memset
#define memcpy hal_memcpy
#define memcmp hal_memcmp
#define strlen hal_strlen
#define strcmp hal_strcmp
#define strncmp hal_strncmp
#define strcpy hal_strcpy
#define strtol hal_strtol
#define strstr hal_strstr
#define sprintf hal_sprintf
#define snprintf hal_snprintf
#define vsnprintf hal_vsnprintf
#define strcoll hal_strcoll
#define abort hal_abort

#define leslog(_mod_name_, _fmt_, ...) \
    wmprintf("[%s] "_fmt_"\r\n", _mod_name_, ##__VA_ARGS__)
#define LES_LOG(...) \
    leslog("LE", ##__VA_ARGS__)
#define LES_LOGW(...) \
    leslog("LE [W]", ##__VA_ARGS__)
#define LES_LOGE(...) \
    leslog("LE [E]", ##__VA_ARGS__)
#define LES_PRINTF(...) \
    wmprintf(__VA_ARGS__)

#if !defined(lua_writestring)
#define lua_writestring(s,l)   wmprintf(s);
#endif

#if !defined(lua_writeline)
#define lua_writeline(...)
#endif

    

#elif defined (LINUX) || defined(__ANDROID__)
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
    
#define LES_LOG(fmt, ...) \
    printf("LE " fmt, ##__VA_ARGS__)
#define LES_LOGW(fmt, ...) \
    printf("LE [W]" fmt, ##__VA_ARGS__)
#define LES_LOGE(fmt, ...) \
    printf("LE [E]" fmt, ##__VA_ARGS__)
#define LES_PRINTF(fmt, ...) \
    printf(fmt, ##__VA_ARGS__)

#if !defined(lua_writestring)
#define lua_writestring(s,l)   printf(s);
#endif

#if !defined(lua_writeline)
#define lua_writeline(...)
#endif

#else
#error ("no adpation...")
// #include <errno.h>
// #include <stdarg.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
#endif

#endif
