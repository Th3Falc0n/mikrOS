#include "stdio.h"

int putc(char c) {
  struct regstate state = {
    .eax = 201,
    .ebx = (uint32_t)c,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);
  
  return state.eax;
}

int puts(const char* cp) {
  struct regstate state = {
    .eax = 202,
    .ebx = (uint32_t)cp,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);
  
  return state.eax;
}

static int putn(unsigned long x, int base)
{
  char buf[65];
  const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  char* p;
  int wrt = 0;

  if (base > 36) {
    return 0;
  }

  p = buf + 64;
  *p = '\0';
  do {
    wrt++;
    *--p = digits[x % base];
    x /= base;
  } while (x);
  puts(p);
  
  return wrt;
}


int printf(const char* fmt, ...)
{
  va_list ap;
  const char* s;
  unsigned long n;
	char c;
	int wrt = 0;

  va_start(ap, fmt);
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 's':
          s = va_arg(ap, char*);
          wrt += puts(s);
          break;
        case 'd':
        case 'u':
          n = va_arg(ap, unsigned long int);
          wrt += putn(n, 10);
          break;
        case 'x':
        case 'p':
          n = va_arg(ap, unsigned long int);
          wrt += putn(n, 16);
          break;
        case 'c':
          c = va_arg(ap, int);
          wrt += putc(c);
          break;
        case '%':
          wrt += putc('%');
          break;
        case '\0':
          goto out;
        default:
          wrt += putc('%');
          wrt += putc(*fmt);
          break;
      }
    } else {
      wrt += putc(*fmt);
    }

    fmt++;
  }

out:
  va_end(ap);

  return wrt;
}
