#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

void _start();
void exit(int returncode);
void yield();
uint32_t fork(void);
void exec(char* path, char** args);

#endif
