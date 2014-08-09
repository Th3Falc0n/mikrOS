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


static void _handle_rpc(uint32_t type, uint32_t funcID, uint32_t datasize) {
    void* localData = malloc(datasize);
    fetch_rpc_data(localData);

    printf("RPC received! type:%x=%x funcID:%x=%x datasize:%x=%x\n", (uint32_t)&type, type, (uint32_t)&funcID, funcID, (uint32_t)&datasize, datasize);

    free(localData);
    return_rpc(0);
}

void set_rpc_handler() {
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
