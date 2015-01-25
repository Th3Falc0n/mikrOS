#include "stdio.h"
#include "string.h"
#include "process.h"

uint32_t getLastVFSErr() {
    struct regstate state = {
        .eax = 15,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}

static HANDLE getpmhandle(uint32_t pmid) {
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

static HANDLE resolveHandle(HANDLE hdl) {
    if(hdl < 0xFFF) {
        hdl = getpmhandle(hdl);
    }
    return hdl;
}

static HANDLE getstdout() { return getpmhandle(PMID_STDOUT); };
static HANDLE getstdin() { return getpmhandle(PMID_STDIN); };
static HANDLE getstderr() { return getpmhandle(PMID_STDERR); };

int fopenpmhandle (uint32_t pmid, char* path, int pid) {
    struct regstate state = {
      .eax = 21,
      .ebx = pmid,
      .ecx = (uint32_t)path,
      .edx = pid,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

int setstdout(char* path) { return fopenpmhandle(PMID_STDOUT, path, 0); };
int setstdin (char* path) { return fopenpmhandle(PMID_STDIN , path, 0); };
int setstderr(char* path) { return fopenpmhandle(PMID_STDERR, path, 0); };

int setpstdout(char* path, int pid) { return fopenpmhandle(PMID_STDOUT, path, pid); };
int setpstdin (char* path, int pid) { return fopenpmhandle(PMID_STDIN , path, pid); };
int setpstderr(char* path, int pid) { return fopenpmhandle(PMID_STDERR, path, pid); };

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

static void printrwerror(HANDLE handle, uint32_t res) {
    if(res == RW_ERR_VFS) {
        printf("HANDLE[0x%x] VFS_ERROR\n", handle);
    }

    if(res == RW_ERR_DRIVER) {
        printf("HANDLE[0x%x] DRIVER_ERROR\n", handle);
    }

    if(res == RW_NOFM_READ) {
        printf("HANDLE[0x%x] FILEMODE doesn't allow read\n", handle);
    }

    if(res == RW_NOFM_WRITE) {
        printf("HANDLE[0x%x] FILEMODE doesn't allow write\n", handle);
    }
}

uint32_t favailable(uint32_t handle) {
    struct regstate state = {
      .eax = 16,
      .ebx = (uint32_t)handle,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
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
    handle = resolveHandle(handle);

    uint32_t res = frwrite(handle, src, length);

    while(res == RW_BLOCK) {
        yield();
        res = frwrite(handle, src, length);
    }

    printrwerror(handle, res);

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
        yield();
        res = frread(handle, dest, length);
    }

    printrwerror(handle, res);

    return res;
}

HANDLE fmkfifo(char* path) {
    struct regstate state = {
      .eax = 14,
      .ebx = (uint32_t)path,
      .ecx = 1,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return (HANDLE)state.eax;
}

void fnomkfifo(char* path) {
    struct regstate state = {
      .eax = 14,
      .ebx = (uint32_t)path,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return;
}

char fgetc(HANDLE hdl) {
    hdl = resolveHandle(hdl);
    char in = 0;
    if(hdl != 0) {
        fread(hdl, &in, sizeof(char));
    }
    return in;
}

char* fgets(char* str, int num, HANDLE hdl) {
    for(int n = 0; n < (num - 1); n++) {
        str[n] = fgetc(hdl);
        if(str[n] == 0 || str[n] == '\0') {
            return str;
        }
    }

    str[num] = '\0';
    return str;
}

char getc(HANDLE hdl) {
    return fgetc(hdl);
}

char getchar() {
    return fgetc(PMID_STDIN);
}

char* getln(char* str) {
    uint32_t index = 0;
    char lastChar = 0;

    while((lastChar = fgetc(PMID_STDIN))) {
        if(lastChar == '\n') {
            putchar('\n');
            break;
        }

        if(lastChar == 8) {
            if(index != 0) {
                index--;
                putchar(8);
            }
            str[index] = 0;
            continue;
        }

        str[index] = lastChar;
        putchar(lastChar);

        index++;
    }

    str[index] = 0;

    return str;
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

int putchar(char c) {
    return fputc(c, PMID_STDOUT);
}

int putc(char c, HANDLE hdl) {
    return fputc(c, hdl);
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
          wrt += putchar(c);
          break;
        case '%':
          wrt += putchar('%');
          break;
        case '\0':
          goto out;
        default:
          wrt += putchar('%');
          wrt += putchar(*fmt);
          break;
      }
    } else {
      wrt += putchar(*fmt);
    }

    fmt++;
  }

out:
  va_end(ap);

  return wrt;
}
