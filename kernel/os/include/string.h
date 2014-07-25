#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void*        memset(void* buf, int c, size_t n);
void*        memcpy(void* dest, const void* src, size_t n);
int          memcmp(const void* ptr1, const void* ptr2, size_t num);

unsigned int strlen(const char* str);
int          strcmp(const char* str1, const char* str2);
char*        strtok(char* str, const char* delimiters);
char*        strcpy(char* dest, const char* src);

#endif

