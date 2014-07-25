#ifndef _PORTS_H_
#define _PORTS_H_

#include "stdint.h"

static inline uint16_t inw(uint16_t _port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

static inline uint8_t inb(uint16_t _port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

static inline uint32_t inl(uint16_t _port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a" (result) : "Nd" (_port));
    return result;
}

static inline void outw(uint16_t _port, uint16_t _data) {
    asm volatile ("outw %0, %1" : : "a" (_data), "Nd" (_port));
}

static inline void outb(uint16_t _port, uint8_t _data) {
    asm volatile ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

static inline void outl(uint16_t _port, uint32_t _data) {
    asm volatile ("outl %0, %1" : : "a"(_data), "Nd" (_port));
}

static inline void outb_wait(uint16_t _port, uint8_t _data) {
    asm volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
}

#endif

