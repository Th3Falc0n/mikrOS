#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"
#include "catofdeath.h"

void init_rpc_call(struct task* task) {
    struct rpc* rpc = task->rpc;

    rpc->state = calloc(1, sizeof(struct cpu_state));
    memcpy(rpc->state, task->cpuState, sizeof(struct cpu_state));

    kprintf("RPC_INIT to %x", task->rpc_handler_addr);

    rpc->state->eip = task->rpc_handler_addr;
    rpc->state->esp -= 4 * sizeof(uint32_t);

    uint32_t* rpch_args = (void*) rpc->state->esp;

    rpch_args[0] = rpc->type;
    rpch_args[1] = rpc->funcID;
    rpch_args[2] = rpc->dataSize;

    rpc->executing = 1;
}

uint32_t create_rpc_call(int dPID, uint32_t type, uint32_t funcID, uint32_t dsize, void* data) {
    struct rpc* nrpc = calloc(1, sizeof(struct rpc));
    struct task* destTask = get_task_by_pid(dPID);

    if(destTask == 0) {
        setclr(0x0A);
        kprintf("Tried to execute RPC in non-existing task.\n");
        setclr(0x07);
        return RPCE_NODEST;
    }

    if(destTask->rpc_handler_addr == 0) {
        setclr(0x0A);
        kprintf("Tried to execute RPC in non RPC enabled task.\n");
        setclr(0x07);
        return RPCE_NOFUNC;
    }

    nrpc->type = type;
    nrpc->funcID = funcID;
    nrpc->returnPID = get_current_task() ? get_current_task()->PID : 0;

    nrpc->data = malloc(dsize);
    memcpy(nrpc->data, data, dsize);
    nrpc->dataSize = dsize;

    if(destTask->rpc == 0) {
        destTask->rpc = nrpc;
        return RPCE_OK;
    }
    else
    {
        struct rpc* ptr = destTask->rpc;

        while(ptr != 0) {
            if(ptr->next == 0) {
                kprintf("Chained RPC occured\n");

                ptr->next = nrpc;
                return RPCE_OK;
            }
            ptr = ptr->next;
        }
    }

    return RPCE_UNKNOWN;
}

struct cpu_state* return_rpc_call(struct cpu_state* cpu) {
    struct task* task = get_current_task();

    if(!task->rpc) {
        setclr(0x04);
        kprintf("\n~~~ Terminated task (PID=%d PATH=%s). RPC_RET_WITHOUT_CALL \n", task->PID, task->filePath);
        kprintf("\n");
        show_dump(cpu);
        setclr(0x07);

        return terminate_current(cpu);
    }

    struct rpc* rpc = task->rpc;
    task->rpc = task->rpc->next;

    if(task->rpc->returnPID) {
        struct task* rTask = get_task_by_pid(rpc->returnPID);

        if(rTask == 0) {
            return schedule_to_task(task);
        }

        rTask->cpuState->eax = RPCE_OK;

        return schedule_to_task(rTask);
    }
    else
    {
        return schedule_to_task(task);
    }
}
