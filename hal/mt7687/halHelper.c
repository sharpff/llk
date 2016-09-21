#include "halHeader.h"
#include "os.h"

void* hal_memset(void *buf, int i, unsigned int len) {
    return os_memset(buf, i, len);
}

void* hal_memcpy(void *dst, const void *src, unsigned int len) {
    return os_memcpy(dst, src, len);
}

int hal_memcmp(const void *buf1, const void *buf2, unsigned int len) {
    return os_memcmp(buf1, buf2, len);
}

unsigned int  hal_strlen(const char *str)  { 
    return os_strlen(str); 
} 

int hal_strcmp(const char *dst, const char *src)  { 
    return os_strcmp(dst, src); 
}

int hal_strncmp(const char *dst, const char *src, unsigned int len)  { 
    return os_strncmp(dst, src, len); 
} 

char * hal_strcpy(char *dst, const char *src)  { 
    return os_strncpy(dst, src, os_strlen(src));
} 

long int hal_strtol(const char *str, char **c, int adecimal) {
    return strtol(str, c, adecimal);
}

char *hal_strstr(const char *haystack, const char *needle) {
    return os_strstr(haystack, needle);
}

char *hal_strrstr(const char *haystack, const char *needle) {
    return os_strrstr(haystack, needle);
}

int hal_sprintf(char *buf, const char *fmt, ...) {
    va_list ap;
    int rv;

    va_start(ap, fmt);
    rv = vsnprintf(buf, 260, fmt, ap);
    va_end(ap);
    return rv;
}

int hal_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    int rv;

    va_start(ap, fmt);
    rv = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return rv;
}

int hal_vsnprintf(char *buffer, size_t n, const char *format, va_list ap) {
    return vsnprintf(buffer, n, format, ap);
}

int hal_strcoll(const char *l, const char *r) {
    return (strlen(l) - strlen(r)); 
}

void hal_abort() {
    for (;;);
}


inline float floorf(float x) {
    return (float)(x < 0.f ? (((int)x) - 1) : ((int)x));
}
inline double floor(double x) {
    return (double)(x < (double)0.f ? (((int)x) - 1) : ((int)x));
}
inline void _exit(int status) {
    // APPLOGE("_exit");
}
typedef int FILEHANDLE;
inline int _close(FILEHANDLE fh) {
    // APPLOGE("_close");
    return 0;
}
inline int _write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) {
    // APPLOGE("_write");
    return 0;
}
inline int _read(FILEHANDLE fh, unsigned char*buf, unsigned len, int mode) {
    // APPLOGE("_read");
    return 0;
}
inline int _lseek(FILEHANDLE fh, long pos) {
    // APPLOGE("_lseek");
    return 0;
}
inline long _fstat(FILEHANDLE fh) {
    // APPLOGE("_fstat");
    return 0;
}
inline int _isatty(FILEHANDLE fh) {
    // APPLOGE("_isatty");
    return 0;
}
inline int _kill(FILEHANDLE fh) {
    // APPLOGE("_kill");
    return 0;
}
inline int _getpid(FILEHANDLE fh) {
    // APPLOGE("_getpid");
    return 0;
}
inline int _sbrk(void) {
    // APPLOGE("_sbrk");
    return 0;
}

