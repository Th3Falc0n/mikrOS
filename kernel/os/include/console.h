#ifndef CONSOLE_H
#define CONSOLE_H

#include "stdarg.h"

void clrscr(void);
void setclr(char color);
int kprintf(const char* fmt, ...);

#endif

