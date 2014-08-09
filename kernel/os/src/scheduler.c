#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"
#include "catofdeath.h"

uint32_t schedulingEnabled = 0;

struct task* first_task = 0;
struct task* current_task = 0;

int nextPID = 1; //FIXME int.max_value overflow

void enableScheduling(void) {
    schedulingEnabled = 1;
}

uint32_t isSchedulingEnabled(void) {
    return schedulingEnabled;
}

struct task* get_current_task(void) {
    return current_task;
}

struct task* get_task_by_pid(int pid) {
    struct task* cur = first_task;

    while(cur != 0 && cur->PID != pid) {
        cur = cur->next;
    }

    return cur;
}

uint32_t register_handle(struct res_handle* h) {
    struct hl_node* old = current_task->handle_list;

    current_task->handle_list = malloc(sizeof(struct hl_node));
    current_task->handle_list->next = old;
    current_task->handle_list->handle = h;

    return 0;
}

uint32_t unregister_handle(struct res_handle* h) {
    struct hl_node* cur = current_task->handle_list;

    if(h == 0) return 2;
    if(cur == 0) return 1;

    if(cur->handle == h) {
        current_task->handle_list = cur->next;
        free(cur);

        return 0;
    }

    while(cur != 0) {
        if(cur->next->handle == h) {
            void* next = cur->next;
            cur->next =  cur->next->next;

            free(next);

            return 0;
        }

        cur = cur->next;
    }

    return 3;
}

struct cpu_state* schedule_exception(struct cpu_state* cpu) {
    if (current_task == first_task && current_task->next == 0) {
        //Only one process is running, which just crashed. Stop system.
        setclr(0x04);
        kprintf("\n~~~ Terminated task (PID=%d PATH=%s) due to exception %x:%x \n", current_task->PID, current_task->filePath, cpu->intr, cpu->error);
        show_cod(cpu, "Last task crashed. Terminating kernel...");

        //will never occur cause COD terminates execution
        return 0;
    } else {
        //Potential security leaks available in following code.
        setclr(0x04);
        kprintf("\n~~~ Terminated task (PID=%d PATH=%s) due to exception %x:%x \n", current_task->PID, current_task->filePath, cpu->intr, cpu->error);
        kprintf("\n");
        show_dump(cpu);
        setclr(0x07);

        return terminate_current(cpu);
    }
}

struct cpu_state* terminate_current(struct cpu_state* cpu) {
    struct task* next = current_task->next;
    struct task* prev = current_task->prev;
    struct task* old = current_task;

    vmm_free_current_pagetables();

    if (current_task == first_task) {
        first_task = current_task->next;
    }

    if (next != 0) {
        next->prev = prev;
    }

    if (prev != 0) {
        prev->next = next;
    }

    if (next == 0)
        next = first_task;

    current_task = next;

    free(old->cpuState);
    free(old);

    if(current_task == 0) {
        show_cod(cpu, "Last task terminated.");
    }

    vmm_activate_pagedir(current_task->phys_pdir);
    return current_task->cpuState;
}

struct task* init_task(uint32_t task_pagedir, void* entry) {
    struct task* ntask = calloc(1, sizeof(struct task));
    ntask->cpuState = calloc(1, sizeof(struct cpu_state));

    ntask->phys_pdir = task_pagedir;
    ntask->user_stack_bottom = (void*) 0xFFFFE000;
    ntask->PID = nextPID++;

    ntask->stdin  = 0;
    ntask->stdout = 0;
    ntask->stderr = 0;

    ntask->next = (void*) 0;
    ntask->prev = (void*) 0;

    if (first_task == 0) {
        first_task = ntask;
    } else {
        ntask->next = first_task;
        first_task->prev = ntask;
        first_task = ntask;
    }

    uint32_t rest_pdir = vmm_get_current_pagedir();
    vmm_activate_pagedir(task_pagedir);

    vmm_alloc_addr(ntask->user_stack_bottom, 0);

    struct cpu_state nstate = { .eax = 0, .ebx = 0, .ecx = 0, .edx = 0,
            .esi = 0, .edi = 0, .ebp = 0, .esp =
                    (uint32_t) ntask->user_stack_bottom + 4096, .eip =
                    (uint32_t) entry,

            /* Ring-3-Segmentregister */
            .cs = 0x18 | 0x03, .ss = 0x20 | 0x03,

            .eflags = 0x200, };

    memcpy(ntask->cpuState, &nstate, sizeof(struct cpu_state));

    vmm_activate_pagedir(rest_pdir);

    return ntask;
}

void save_cpu_state(struct cpu_state* cpu) {
    if(current_task->rpc && current_task->rpc->executing) {
        memcpy(current_task->rpc->state, cpu, sizeof(struct cpu_state));
    }
    else
    {
        memcpy(current_task->cpuState, cpu, sizeof(struct cpu_state));
    }
}

struct cpu_state* schedule_to_task(struct task* dest) {
    current_task = dest;

    vmm_activate_pagedir(dest->phys_pdir);

    if(!dest->rpc) {
        return dest->cpuState;
    }
    else
    {
        if(!dest->rpc->executing) {
            kprintf("RPC_INIT\n");
            init_rpc_call(dest);
        }
        kprintf("RPC_SCHED at %x\n", dest->rpc->state->eip);
        return dest->rpc->state;
    }
}

struct cpu_state* schedule(struct cpu_state* cpu) {
    if (first_task != 0 && schedulingEnabled) {
        if (current_task == 0) {
            current_task = first_task;
            vmm_activate_pagedir(current_task->phys_pdir);
            return current_task->cpuState;
        }

        struct task* next = current_task->next;
        if (next == 0)
            next = first_task;

        save_cpu_state(cpu);

        return schedule_to_task(next);
    }
    return cpu;
}

