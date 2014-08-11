#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

#define RPCT_IRQ     0
#define RPCT_KERNEL  1
#define RPCT_U2U     2

void     _start();
void     exit(int returncode);
uint32_t pexists(int pid);
void     yield();
void     getExecPath(char* dest);
int      changeExecPath(char* path);
void     cd(char* path);
int      exec(char* path, char** args);
int      texec(char* path, char** args);
void     printFilesystemError(char* path, uint32_t code);
uint32_t call_rpc(int pid, uint32_t funcID, uint32_t datasize, void* data);
uint32_t register_irq_handler(uint32_t irq, void(*handler)(uint32_t));

#endif
