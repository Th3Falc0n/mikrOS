#include "ports.h"
#include "syscall.h"

uint32_t require_port(uint16_t port) {
    struct regstate state = {
        .eax = 50,
        .ebx = (uint32_t) port,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}

uint32_t port_out(uint32_t mode, uint16_t port, uint32_t value) {
    struct regstate state = {
        .eax = 51,
        .ebx = mode,
        .ecx = (uint32_t) port,
        .edx = value,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}

uint32_t port_in(uint32_t mode, uint16_t port) {
    struct regstate state = {
        .eax = 52,
        .ebx = mode,
        .ecx = (uint32_t) port,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}
