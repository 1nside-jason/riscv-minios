// include/string.h
#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>
#include "riscv.h"

void* memcpy(void *dst, const void *src, uint64_t n);
void* memset(void *dst, int c, uint64_t n);
int strcmp(const char *s1, const char *s2);
char* strcpy(char *dst, const char *src);
size_t strlen(const char *s);
char* strchr(const char *s, int c);

#endif
