#include "console.h"

static int x = 0;
static int y = 0;
static int kprintf_res = 0;
static char color = 0x07;

static char* video = (char*) 0xb8000;

static void kputc(char c)
{
    if ((c == '\n') || (x > 79)) {
        x = 0;
        y++;
    }

    if (c == '\n') {
        return;
    }

    if (y > 24) {
        int i;
        for (i = 0; i < 2 * 24 * 80; i++) {
            video[i] = video[i + 160];
        }

        for (; i < 2 * 25 * 80; i++) {
            video[i] = 0;
        }
        y--;
    }

    video[2 * (y * 80 + x)] = c;
    video[2 * (y * 80 + x) + 1] = color;

    x++;
    kprintf_res++;
}

static void kputs(const char* s)
{
    while (*s) {
        kputc(*s++);
    }
}

static void kputn(unsigned long x, int base)
{
    char buf[65];
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p;

    if (base > 36) {
        return;
    }

    p = buf + 64;
    *p = '\0';
    do {
        *--p = digits[x % base];
        x /= base;
    } while (x);
    kputs(p);
}

void clrscr(void)
{
    int i;
    for (i = 0; i < 2 * 25 * 80; i++) {
        video[i] = 0;
    }
    
    color = 0x07;

    x = y = 0;
}

void setclr(char clr) {
  color = clr;
}

int kprintf(const char* fmt, ...)
{
  va_list ap;
  const char* s;
  unsigned long n;
	char c;

  va_start(ap, fmt);
  kprintf_res = 0;
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 's':
          s = va_arg(ap, char*);
          kputs(s);
          break;
        case 'd':
        case 'u':
          n = va_arg(ap, unsigned long int);
          kputn(n, 10);
          break;
        case 'x':
        case 'p':
          n = va_arg(ap, unsigned long int);
          kputn(n, 16);
          break;
        case 'c':
          c = va_arg(ap, int);
          kputc(c);
          break;
        case '%':
          kputc('%');
          break;
        case '\0':
          goto out;
        default:
          kputc('%');
          kputc(*fmt);
          break;
      }
    } else {
      kputc(*fmt);
    }

    fmt++;
  }

out:
  va_end(ap);

  return kprintf_res;
}

