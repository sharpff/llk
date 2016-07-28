#include "halHeader.h"

void* hal_memset(void *buf, int i, unsigned int len) {
}

void* hal_memcpy(void *dst, const void *src, unsigned int len) {
}

int hal_memcmp(const void *buf1, const void *buf2, unsigned int len) {
}

unsigned int  hal_strlen(const char *str)  { 
} 

int hal_strcmp(const char *dst, const char *src)  { 
}

int hal_strncmp(const char *dst, const char *src, unsigned int len)  {  
} 

char * hal_strcpy(char *dst, const char *src)  { 
} 

long int hal_strtol(const char *str, char **c, int adecimal) {
}

char *hal_strstr(const char *haystack, const char *needle) {
}

int hal_sprintf(char *buf, const char *fmt, ...) {
    va_list ap;
    int rv;

    va_start(ap, fmt);
    rv = vsnprintf(buf, ~(size_t) 0, fmt, ap);
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

