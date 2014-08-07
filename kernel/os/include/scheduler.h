#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"
#include "stdlib.h"
#include "vmm.h"

#define HANDLE_COUNT 1024

#define PMID_STDOUT  0
#define PMID_STDIN   1
#define PMID_STDERR  2

struct task {
    int PID;
    struct cpu_state* cpuState;
    struct task* next;
    struct task* prev;
    uint8_t* user_stack_bottom;
    uint32_t phys_pdir;

    char** args;
    char*  filePath;
    char*  execPath;

    uint32_t vfserr;

    struct res_handle* stdout;
    struct res_handle* stderr;
    struct res_handle* stdin;

    struct hl_node* handle_list;
};

struct hl_node {
    struct res_handle* handle;
    struct hl_node* next;
};


struct task* init_task(uint32_t task_pagedir, void* entry);
struct task* get_current_task(void);
uint32_t register_handle(struct res_handle* h);
uint32_t unregister_handle(struct res_handle* h);
struct cpu_state* terminate_current(struct cpu_state* cpu);
struct cpu_state* schedule_exception(struct cpu_state* cpu);
struct cpu_state* schedule(struct cpu_state* cpu);
void save_cpu_state(struct cpu_state* cpu);
void enableScheduling(void);
uint32_t isSchedulingEnabled(void);

#endif
