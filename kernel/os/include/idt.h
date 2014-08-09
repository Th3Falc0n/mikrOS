#ifndef IDT_H
#define IDT_H

#include "stdint.h"

#define IDT_FLAG_INTERRUPT_GATE 0xe
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING3 0x60

void init_idt();
void register_intr_handler(int i, void (*fn)());
uint32_t register_irq_rpc(uint32_t irq);
void disable_irq_rpc(uint32_t irq);
void enable_irq_rpc(uint32_t irq);

#endif
