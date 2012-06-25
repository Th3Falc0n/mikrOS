#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"

struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

struct task* init_task(void* entry);
struct cpu_state* schedule(struct cpu_state* cpu);

#endif
