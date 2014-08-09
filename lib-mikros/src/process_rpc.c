#include "process.h"
#include "stdio.h"
#include "stdlib.h"

static void return_rpc(int code) {
    struct regstate state = {
      .eax = 30,
      .ebx = (uint32_t)code,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    while(1);
}

uint32_t call_rpc(int pid, uint32_t funcID, uint32_t datasize, void* data) {
    struct regstate state = {
      .eax = 31,
      .ebx = (uint32_t)pid,
      .ecx = funcID,
      .edx = datasize,
      .esi = (uint32_t)data,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
}

void fetch_rpc_data(void* dest) {
    struct regstate state = {
      .eax = 33,
      .ebx = (uint32_t)dest,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);
}

static uint32_t register_irq_rpc_syscall(uint32_t irq) {
    struct regstate state = {
      .eax = 40,
      .ebx = irq,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
}

static void (*irq_handlers[128])(uint32_t);

uint32_t register_irq_handler(uint32_t irq, void(*handler)(uint32_t)) {
    if(register_irq_rpc_syscall(irq)) {
        irq_handlers[irq] = handler;
        return 1;
    }
    return 0;
}

static void _handle_rpc(uint32_t type, uint32_t funcID, uint32_t datasize) {
    void* localData = malloc(datasize);
    fetch_rpc_data(localData);

    if(type == RPCT_IRQ) {
        if(irq_handlers[funcID]) {
            irq_handlers[funcID](funcID);
            return_rpc(0);
        }
    }

    printf("Unhandled RPC!\n");

    free(localData);
    return_rpc(0);
}

void set_rpc_handler() {
    memset(irq_handlers, 0, sizeof(irq_handlers));

    struct regstate state = {
      .eax = 32,
      .ebx = (uint32_t)_handle_rpc,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);
}
