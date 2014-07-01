#ifndef STDIO_H
#define STDIO_H

#include "syscall.h"
#include "stdint.h"
#include "stdarg.h"

int putc(char c);
int puts(const char* cp);
int printf(const char* fmt, ...);

#endif
