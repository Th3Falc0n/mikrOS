#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdint.h"

struct regstate {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
};

void syscall(struct regstate* state);

#endif
