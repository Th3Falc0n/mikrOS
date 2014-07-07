#ifndef VMMCALL_H
#define VMMCALL_H

#include "stdint.h"
#include "syscall.h"

void*    vmm_alloc_cont (uint32_t cont);
void     vmm_free       (void* vaddr);

#endif
