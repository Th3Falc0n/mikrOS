#include "ports.h"
#include "scheduler.h"

void free_ports_for_pid(int pid) {
    for(int i = 0; i < 65536; i++) {
        if(acquired_ports[i] == pid) {
            acquired_ports[i] = 0;
        }
    }
}

uint32_t require_port(uint16_t port) {
    if(acquired_ports[port] != 0) return 0;
    acquired_ports[port] = get_current_task()->PID;
    return 1;
}

uint32_t port_in(uint32_t mode, uint16_t port) {
    if(acquired_ports[port] != get_current_task()->PID) {
        kprintf("Denied PIN for task %d at port %x\n", get_current_task()->PID, port);
        while(1);
        return 0;
    }

    switch(mode) {
    case PORTM_BYTE:
        return (uint32_t)inb(port);
    case PORTM_WORD:
        return (uint32_t)inw(port);
    case PORTM_LONG:
        return inl(port);
    }

    return 0;
}

uint32_t port_out(uint32_t mode, uint16_t port, uint32_t value) {
    if(acquired_ports[port] != get_current_task()->PID) {
        return 0;
    }

    switch(mode) {
    case PORTM_BYTE:
        outb(port, (uint8_t)value);
        return 1;
    case PORTM_WORD:
        outw(port, (uint16_t)value);
        return 1;
    case PORTM_LONG:
        outl(port, value);
        return 1;
    }

    return 0;
}

uint16_t inw(uint16_t _port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

uint8_t inb(uint16_t _port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

uint32_t inl(uint16_t _port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

void outw(uint16_t _port, uint16_t _data) {
    asm volatile ("outw %0, %1" : : "a" (_data), "Nd" (_port));
}

void outb(uint16_t _port, uint8_t _data) {
    asm volatile ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

void outl(uint16_t _port, uint32_t _data) {
    asm volatile ("outl %0, %1" : : "a"(_data), "Nd" (_port));
}

void outb_wait(uint16_t _port, uint8_t _data) {
    asm volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
}
