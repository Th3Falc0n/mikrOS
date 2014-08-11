#ifndef _PORTS_H_
#define _PORTS_H_

#include "stdint.h"

#define PORTM_BYTE 0
#define PORTM_WORD 1
#define PORTM_LONG 2

int acquired_ports[65536];

void     free_ports_for_pid(int pid);
uint32_t require_port(uint16_t port);

uint32_t port_in(uint32_t mode, uint16_t port);
uint32_t port_out(uint32_t mode, uint16_t port, uint32_t value);

uint8_t  inb(uint16_t _port);
uint16_t inw(uint16_t _port);
uint32_t inl(uint16_t _port);

void outw(uint16_t _port, uint16_t _data);
void outb(uint16_t _port, uint8_t _data);
void outl(uint16_t _port, uint32_t _data);
void outb_wait(uint16_t _port, uint8_t _data);

#endif

