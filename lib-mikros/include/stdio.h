#ifndef STDIO_H
#define STDIO_H

#include "syscall.h"
#include "stdint.h"
#include "stdarg.h"

typedef uint32_t HANDLE;

#define PMID_STDOUT  1
#define PMID_STDIN   2
#define PMID_STDERR  3

#define FM_READ   (1 << 0)
#define FM_WRITE  (1 << 1)
#define FM_EXEC   (1 << 2)
#define FM_CREATE (1 << 3)

#define RW_OK         0
#define RW_BLOCK      1
#define RW_ERR_VFS    2
#define RW_ERR_DRIVER 3
#define RW_NOFM_READ  4
#define RW_NOFM_WRITE 5
#define RW_EOF        6

#define PE_NO_ERROR       0
#define PE_FILE_NOT_FOUND 1
#define PE_PERM_DENIED    2
#define PE_CORRUPT_FILE   3
#define PE_FILESYSTEM     4
#define PE_INVALID        5

uint32_t getLastVFSErr();

uint32_t favailable(uint32_t handle);
HANDLE   fopen  (char* path, uint32_t filemode);
HANDLE   fmkfifo(char* path);
int      fclose (HANDLE handle);
uint32_t fwrite (HANDLE handle, const void* src , uint32_t length);
uint32_t fread  (HANDLE handle, void* dest, uint32_t length);

int    fopenpmhandle (uint32_t pmid, char* path, int pid);

int setstdout(char* path);
int setstdin (char* path);
int setstderr(char* path);

int setpstdout(char* path, int pid);
int setpstdin (char* path, int pid);
int setpstderr(char* path, int pid);

int putchar(char c);
int puts(const char* cp);
int printf(const char* fmt, ...);
int putc(char c, HANDLE hdl);
int fputc(char c, HANDLE hdl);
int fputs(const char* cp, HANDLE hdl);

char  fgetc  (HANDLE hdl);
char* fgets  (char* str, int num, HANDLE hdl);
char  getc   (HANDLE hdl);
char  getchar();
char* getln  (char* str);
#endif
