#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

void     _start();
void     exit(int returncode);
void     yield();
void     getExecPath(char* dest);
int      changeExecPath(char* path);
void     cd(char* path);
int      exec(char* path, char** args);
int      texec(char* path, char** args);
void     printFilesystemError(char* path, uint32_t code);
uint32_t call_rpc(int pid, uint32_t funcID, uint32_t datasize, void* data);

#endif
