#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

void exit(int returncode);
uint32_t fork(void);

#endif
