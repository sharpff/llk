/*
 *  Platform abstraction layer
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#include "lprefix.h"
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)

#include "mbedtls/platform.h"


#if defined(MBEDTLS_PLATFORM_MEMORY)

#if !defined(MBEDTLS_PLATFORM_STD_CALLOC)
// LELINK ADAPTOR
static void *platform_calloc_uninit( size_t n, size_t size )
{
    void *p = (void *)halCalloc(n, size);
    return p;
}

#define MBEDTLS_PLATFORM_STD_CALLOC   platform_calloc_uninit
#endif /* !MBEDTLS_PLATFORM_STD_CALLOC */

#if !defined(MBEDTLS_PLATFORM_STD_FREE)
// LELINK ADAPTOR
static void platform_free_uninit(void *ptr)
{
    halFree(ptr);
}


#define MBEDTLS_PLATFORM_STD_FREE     platform_free_uninit
#endif /* !MBEDTLS_PLATFORM_STD_FREE */

void * (*mbedtls_calloc)( size_t, size_t ) = MBEDTLS_PLATFORM_STD_CALLOC;
void (*mbedtls_free)( void * )     = MBEDTLS_PLATFORM_STD_FREE;

int mbedtls_platform_set_calloc_free( void * (*calloc_func)( size_t, size_t ),
                              void (*free_func)( void * ) )
{
    mbedtls_calloc = calloc_func;
    mbedtls_free = free_func;
    return( 0 );
}
#endif /* MBEDTLS_PLATFORM_MEMORY */

#if defined(_WIN32)
#include <stdarg.h>
int mbedtls_platform_win32_snprintf( char *s, size_t n, const char *fmt, ... )
{
    int ret;
    va_list argp;

    /* Avoid calling the invalid parameter handler by checking ourselves */
    if( s == NULL || n == 0 || fmt == NULL )
        return( -1 );

    va_start( argp, fmt );
#if defined(_TRUNCATE)
    ret = _vsnprintf_s( s, n, _TRUNCATE, fmt, argp );
#else
    ret = _vsnprintf( s, n, fmt, argp );
    if( ret < 0 || (size_t) ret == n )
    {
        s[n-1] = '\0';
        ret = -1;
    }
#endif
    va_end( argp );

    return( ret );
}
#endif

#if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
#if !defined(MBEDTLS_PLATFORM_STD_SNPRINTF)
/*
 * Make dummy function to prevent NULL pointer dereferences
 */
static int platform_snprintf_uninit( char * s, size_t n,
                                     const char * format, ... )
{
    ((void) s);
    ((void) n);
    ((void) format);
    return( 0 );
}

#define MBEDTLS_PLATFORM_STD_SNPRINTF    platform_snprintf_uninit
#endif /* !MBEDTLS_PLATFORM_STD_SNPRINTF */

int (*mbedtls_snprintf)( char * s, size_t n,
                          const char * format,
                          ... ) = MBEDTLS_PLATFORM_STD_SNPRINTF;

int mbedtls_platform_set_snprintf( int (*snprintf_func)( char * s, size_t n,
                                                 const char * format,
                                                 ... ) )
{
    mbedtls_snprintf = snprintf_func;
    return( 0 );
}
#endif /* MBEDTLS_PLATFORM_SNPRINTF_ALT */

#if defined(MBEDTLS_PLATFORM_PRINTF_ALT)
#if !defined(MBEDTLS_PLATFORM_STD_PRINTF)
#include <stdarg.h>
/*
 * Make dummy function to prevent NULL pointer dereferences
 */

// LELINK ADAPTOR
static int platform_printf_uninit( const char *format, ... )
{
    int ret = 0;
    // va_list args;
    // va_start(args, format);
    // ret = vsnprintf(wmstdio_msg_buf_impl, sizeof(wmstdio_msg_buf_impl), &format[0], args);
    // va_end(args);
    // wmprintf(wmstdio_msg_buf_impl);
    va_list args;
    va_start(args, format);
    printOut(format, args);
    va_end(args);
    return ret;
}

#define MBEDTLS_PLATFORM_STD_PRINTF    platform_printf_uninit
#endif /* !MBEDTLS_PLATFORM_STD_PRINTF */

int (*mbedtls_printf)( const char *, ... ) = MBEDTLS_PLATFORM_STD_PRINTF;

int mbedtls_platform_set_printf( int (*printf_func)( const char *, ... ) )
{
    mbedtls_printf = printf_func;
    return( 0 );
}
#endif /* MBEDTLS_PLATFORM_PRINTF_ALT */

#if defined(MBEDTLS_PLATFORM_FPRINTF_ALT)
#if !defined(MBEDTLS_PLATFORM_STD_FPRINTF)
/*
 * Make dummy function to prevent NULL pointer dereferences
 */
static int platform_fprintf_uninit( FILE *stream, const char *format, ... )
{
    ((void) stream);
    ((void) format);
    return( 0 );
}

#define MBEDTLS_PLATFORM_STD_FPRINTF   platform_fprintf_uninit
#endif /* !MBEDTLS_PLATFORM_STD_FPRINTF */

int (*mbedtls_fprintf)( FILE *, const char *, ... ) =
                                        MBEDTLS_PLATFORM_STD_FPRINTF;

int mbedtls_platform_set_fprintf( int (*fprintf_func)( FILE *, const char *, ... ) )
{
    mbedtls_fprintf = fprintf_func;
    return( 0 );
}
#endif /* MBEDTLS_PLATFORM_FPRINTF_ALT */

#if defined(MBEDTLS_PLATFORM_EXIT_ALT)
#if !defined(MBEDTLS_PLATFORM_STD_EXIT)
/*
 * Make dummy function to prevent NULL pointer dereferences
 */
static void platform_exit_uninit( int status )
{
    ((void) status);
}

#define MBEDTLS_PLATFORM_STD_EXIT   platform_exit_uninit
#endif /* !MBEDTLS_PLATFORM_STD_EXIT */

void (*mbedtls_exit)( int status ) = MBEDTLS_PLATFORM_STD_EXIT;

int mbedtls_platform_set_exit( void (*exit_func)( int status ) )
{
    mbedtls_exit = exit_func;
    return( 0 );
}
#endif /* MBEDTLS_PLATFORM_EXIT_ALT */

#endif /* MBEDTLS_PLATFORM_C */
