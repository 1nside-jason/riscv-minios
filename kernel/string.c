// kernel/string.c
#include "riscv.h"

void* memcpy(void *dst, const void *src, uint64_t n) {
    char *d = (char*)dst;
    const char *s = (const char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void* memset(void *dst, int c, uint64_t n) {
    char *d = (char*)dst;
    while (n--) {
        *d++ = c;
    }
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcpy(char *dst, const char *src) {
    char *ret = dst;
    while ((*dst++ = *src++) != '\0')
        ;
    return ret;
}

size_t strlen(const char *s) {
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

char* strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c)
            return (char*)s;
        s++;
    }
    if (c == '\0')
        return (char*)s;
    return 0;
}
