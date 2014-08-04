#ifndef STDIO_H
#define STDIO_H

#include "syscall.h"
#include "stdint.h"
#include "stdarg.h"

typedef uint32_t HANDLE;

#define PMID_STDOUT  0
#define PMID_STDIN   1
#define PMID_STDERR  2

#define FM_READ   (1 << 0)
#define FM_WRITE  (1 << 1)
#define FM_EXEC   (1 << 2)
#define FM_CREATE (1 << 3)

#define RW_OK         0
#define RW_BLOCK      1
#define RW_ERR_VFS    2
#define RW_ERR_DRIVER 3

HANDLE   fopen  (char* path, uint32_t filemode);
HANDLE   fmkfifo(char* path);
int      fclose (HANDLE handle);
uint32_t fwrite (HANDLE handle, void* src , uint32_t length);
uint32_t fread  (HANDLE handle, void* dest, uint32_t length);

int    fopenpmhandle (uint32_t pmid, char* path);

int setstdout(char* path);
int setstdin (char* path);
int setstderr(char* path);

int kputc(char c);
int kputs(const char* cp);
int kprintf(const char* fmt, ...);

#endif
