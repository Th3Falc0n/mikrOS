#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"
#include "vmm.h"

struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

uint32_t init_task(uint32_t task_pagedir, void* entry);
struct cpu_state* schedule(struct cpu_state* cpu);
void enable_scheduling(void);

#endif
