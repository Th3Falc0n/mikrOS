#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"

static struct task* first_task = NULL;
static struct task* current_task = NULL;

struct task* init_task(void* entry)
{
    uint8_t* stack = pmm_alloc();
    uint8_t* user_stack = pmm_alloc();

		//kprintf("entry: %x", (uint32_t) entry);

    struct cpu_state new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        .esp = (uint32_t) user_stack + 4096,
        .eip = (uint32_t) entry,

        .cs  = 0x18 | 0x03,
        .ss  = 0x20 | 0x03,

        .eflags = 0x200,
    };

    struct cpu_state* state = (void*) (stack + 4096 - sizeof(new_state));
    *state = new_state;

    struct task* task = pmm_alloc();
    task->cpu_state = state;
    task->next = first_task;
    first_task = task;
    return task;
}

struct cpu_state* schedule(struct cpu_state* cpu)
{
    if (current_task != NULL) {
        current_task->cpu_state = cpu;
    }

    if (current_task == NULL) {
        current_task = first_task;
    } else {
        current_task = current_task->next;
        if (current_task == NULL) {
            current_task = first_task;
        }
    }  

    cpu = current_task->cpu_state;

    return cpu;
}

