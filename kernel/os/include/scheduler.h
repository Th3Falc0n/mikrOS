#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"
#include "stdlib.h"
#include "vmm.h"

struct task {
	int PID;
	struct cpu_state* cpu_state;
	struct task* next;
	struct task* prev;
	uint8_t* user_stack_bottom;
	uint32_t phys_pdir;
};

struct task* init_task(uint32_t task_pagedir, void* entry);
struct cpu_state* terminate_current(struct cpu_state* cpu);
struct cpu_state* schedule_exception(struct cpu_state* cpu);
struct cpu_state* schedule(struct cpu_state* cpu);
void save_cpu_state(struct cpu_state* cpu);
void fork_task_state(struct task* new_task);
void enable_scheduling(void);
uint32_t scheduling_enabled(void);

#endif
