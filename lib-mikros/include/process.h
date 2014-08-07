#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

#define EXEC_OK             0
#define EXEC_FILE_NOT_FOUND 1
#define EXEC_PERM_DENIED    2
#define EXEC_CORRUPT_ELF    3
#define EXEC_FILESYSTEM     4

void _start();
void exit(int returncode);
void yield();
int  exec(char* path, char** args);
void texec(char* path, char** args);
void printexecerror(char* path, uint32_t code);

#endif
