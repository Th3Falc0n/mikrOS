#include "stdio.h"
#include "string.h"

static HANDLE getpmhandle   (uint32_t pmid) {
    struct regstate state = {
      .eax = 20,
      .ebx = pmid,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

static HANDLE getstdout() { return getpmhandle(PMID_STDOUT); };
static HANDLE getstdin() { return getpmhandle(PMID_STDIN); };
static HANDLE getstderr() { return getpmhandle(PMID_STDERR); };

int fopenpmhandle (uint32_t pmid, char* path) {
    struct regstate state = {
      .eax = 21,
      .ebx = pmid,
      .ecx = (uint32_t)path,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

int setstdout(char* path) { return fopenpmhandle(PMID_STDOUT, path); };
int setstdin (char* path) { return fopenpmhandle(PMID_STDIN , path); };
int setstderr(char* path) { return fopenpmhandle(PMID_STDERR, path); };

HANDLE fopen(char* path, uint32_t mode) {
    struct regstate state = {
      .eax = 10,
      .ebx = (uint32_t)path,
      .ecx = mode,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

int fclose(uint32_t handle) {
    struct regstate state = {
      .eax = 11,
      .ebx = (uint32_t)handle,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (int)state.eax;
}


static uint32_t frwrite(uint32_t handle, const void* src, uint32_t length) {
    struct regstate state = {
      .eax = 12,
      .ebx = (uint32_t)handle,
      .ecx = (uint32_t)src,
      .edx = length,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (int)state.eax;
}

uint32_t fwrite(uint32_t handle, const void* src, uint32_t length) {
    uint32_t res = frwrite(handle, src, length);

    while(res == RW_BLOCK) {
        res = frwrite(handle, src, length);
    }

    return res;
}

static uint32_t frread(uint32_t handle, void* dest, uint32_t length) {
    struct regstate state = {
      .eax = 13,
      .ebx = (uint32_t)handle,
      .ecx = (uint32_t)dest,
      .edx = length,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (int)state.eax;
}

uint32_t fread(uint32_t handle, void* dest, uint32_t length) {
    uint32_t res = frread(handle, dest, length);

    while(res == RW_BLOCK) {
        res = frread(handle, dest, length);
    }

    return res;
}

HANDLE fmkfifo(char* path) {
    struct regstate state = {
      .eax = 14,
      .ebx = (uint32_t)path,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

static HANDLE resolveHandle(HANDLE hdl) {
    if(hdl < 0xFFF) {
        hdl = getpmhandle(hdl);
    }
    return hdl;
}

static int kputc(char c) {
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

int putc(char c) {
    return fputc(c, PMID_STDOUT);
}

int fputc(char c, HANDLE hdl) {
    hdl = resolveHandle(hdl);
    if(hdl != 0) {
        return fwrite(hdl, &c, sizeof(char));
    }
    return kputc(c);
}

static int kputs(const char* cp) {
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

int puts(const char* c) {
    return fputs(c, PMID_STDOUT);
}

int fputs(const char* c, HANDLE hdl) {
    hdl = resolveHandle(hdl);
    if(hdl != 0) {
        return fwrite(hdl, c, strlen(c));
    }
    return kputs(c);
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
