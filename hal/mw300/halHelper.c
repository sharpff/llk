#include "leconfig.h"
#include "halHeader.h"


inline void* hal_memset(void *buf, int i, unsigned int len)
{
    return memset(buf, i, len);
}

inline void* hal_memcpy(void *dst, const void *src, unsigned int len)
{
    return memcpy(dst, src, len);
}

inline int hal_memcmp(const void *buf1, const void *buf2, unsigned int len)
{
    return memcmp(buf1, buf2, len);
}

inline unsigned int  hal_strlen(const char *str) 
{ 
    return strlen(str); 
} 

inline int hal_strcmp(const char *dst, const char *src) 
{ 
    return strcmp(dst, src); 
} 

inline int hal_strncmp(const char *dst, const char *src, unsigned int len) 
{ 
    return strncmp(dst, src, len); 
} 

inline char * hal_strcpy(char *dst, const char *src) 
{ 
    return strcpy(dst, src); 
} 

inline long int hal_strtol(const char *str, char **c, int adecimal)
{
    return strtol(str, c, adecimal);
}

inline char *hal_strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

inline int hal_sprintf(char *buf, const char *fmt, ...) {
    va_list ap;
    int rv;

    va_start(ap, fmt);
    rv = vsnprintf(buf, ~(size_t) 0, fmt, ap);
    va_end(ap);
    return rv;
}

inline int hal_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    int rv;

    va_start(ap, fmt);
    rv = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return rv;
}

inline int hal_vsnprintf(char *buffer, size_t n, const char *format, va_list ap) {
    return vsnprintf(buffer, n, format, ap);
}

inline int hal_strcoll(const char *l, const char *r) {
    return (strlen(l) - strlen(r)); 
}

inline void hal_abort() {
    for (;;);
}


inline float floorf(float x) {
    return (float)(x < 0.f ? (((int)x) - 1) : ((int)x));
}
inline double floor(double x) {
    return (double)(x < 0.f ? (((int)x) - 1) : ((int)x));
}
inline void _exit(int status) {
    APPLOGE("_exit");
}
typedef int FILEHANDLE;
inline int _close(FILEHANDLE fh) {
    APPLOGE("_close");
    return 0;
}
inline int _write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) {
    APPLOGE("_write");
    return 0;
}
inline int _read(FILEHANDLE fh, unsigned char*buf, unsigned len, int mode) {
    APPLOGE("_read");
    return 0;
}
inline int _lseek(FILEHANDLE fh, long pos) {
    APPLOGE("_lseek");
    return 0;
}
inline long _fstat(FILEHANDLE fh) {
    APPLOGE("_fstat");
    return 0;
}
inline int _isatty(FILEHANDLE fh) {
    APPLOGE("_isatty");
    return 0;
}
inline int _kill(FILEHANDLE fh) {
    APPLOGE("_kill");
    return 0;
}
inline int _getpid(FILEHANDLE fh) {
    APPLOGE("_getpid");
    return 0;
}
inline int _sbrk(void) {
    APPLOGE("_sbrk");
    return 0;
}

