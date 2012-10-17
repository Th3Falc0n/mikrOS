#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"
#include "vmm.h"

struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

struct task* init_task(void* entry, struct vmm_context* context);
struct cpu_state* schedule(struct cpu_state* cpu);

#endif
