#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"
#include "stdlib.h"
#include "vmm.h"

#define HANDLE_COUNT 1024

#define PMID_STDOUT  1
#define PMID_STDIN   2
#define PMID_STDERR  3

#define RPCT_IRQ     0
#define RPCT_KERNEL  1
#define RPCT_U2U     2

#define RPCE_OK      0
#define RPCE_UNKNOWN 1
#define RPCE_NODEST  2
#define RPCE_NOFUNC  3

struct rpc {
    int returnPID;
    uint32_t type;
    uint32_t funcID;

    void* data;
    uint32_t dataSize;

    uint8_t executing;

    struct cpu_state* state;

    struct rpc* next;
};

struct task {
    int PID;
    struct cpu_state* cpuState;
    struct task* next;
    struct task* prev;
    uint8_t* user_stack_bottom;
    uint32_t phys_pdir;

    struct rpc* rpc;
    uint32_t rpc_handler_addr;

    char** args;
    char*  filePath;
    char*  execPath;

    uint32_t vfserr;

    struct res_handle* stdout;
    struct res_handle* stderr;
    struct res_handle* stdin;

    struct hl_node* handle_list;

    struct task* sub_of;
    struct task* blocked_by_sub;
    struct task* child_of;
};

struct hl_node {
    struct res_handle* handle;
    struct hl_node* next;
};


struct task*      init_task(uint32_t task_pagedir, void* entry);
struct task*      get_current_task(void);
struct task*      get_task_by_pid(int pid);

uint32_t          create_rpc_call(int dPID, uint32_t type, uint32_t funcID, uint32_t dsize, void* data);
struct cpu_state* return_rpc_call(struct cpu_state* cpu);
void              init_rpc_call(struct task* task);

uint32_t          request_irq_rpc(uint32_t irqno);

uint32_t          register_handle(struct res_handle* h);
uint32_t          unregister_handle(struct res_handle* h);

struct cpu_state* terminate_current(struct cpu_state* cpu);
struct cpu_state* schedule_exception(struct cpu_state* cpu);
struct cpu_state* schedule_to_task(struct task* task);
struct cpu_state* schedule(struct cpu_state* cpu);
struct cpu_state* save_cpu_state(struct cpu_state* cpu);

void              enableScheduling(void);
uint32_t          isSchedulingEnabled(void);

#endif
