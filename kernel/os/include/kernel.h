#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"
#include "ports.h"
#include "console.h"
#include "multiboot.h"
#include "pmm.h"
#include "string.h"
#include "elf.h"
#include "vmm.h"
#include "scheduler.h"
#include "idt.h"

struct cpu_state {
    uint32_t   eax;
    uint32_t   ebx;
    uint32_t   ecx;
    uint32_t   edx;
    uint32_t   esi;
    uint32_t   edi;
    uint32_t   ebp;

    uint32_t   intr;
    uint32_t   error;

    uint32_t   eip;
    uint32_t   cs;
    uint32_t   eflags;
    uint32_t   esp;
    uint32_t   ss;
};

void kernel_main();
struct cpu_state* syscall(struct cpu_state* cpu);

#endif
