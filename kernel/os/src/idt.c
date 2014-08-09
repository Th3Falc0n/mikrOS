#include "idt.h"
#include "console.h"
#include "ports.h"
#include "kernel.h"
#include "scheduler.h"
#include "gdt.h"
#include "catofdeath.h"

#define IDT_ENTRIES 64

extern void interrupt_stack;

static uint32_t tss[32] = { 0, (uint32_t) &interrupt_stack, 0x10 };

static uint64_t idt[IDT_ENTRIES];
static void (*handlers[IDT_ENTRIES])();
static int handler_set[IDT_ENTRIES];

static int irq_rpcs[IDT_ENTRIES];

extern void intr_stub_0(void);
extern void intr_stub_1(void);
extern void intr_stub_2(void);
extern void intr_stub_3(void);
extern void intr_stub_4(void);
extern void intr_stub_5(void);
extern void intr_stub_6(void);
extern void intr_stub_7(void);
extern void intr_stub_8(void);
extern void intr_stub_9(void);
extern void intr_stub_10(void);
extern void intr_stub_11(void);
extern void intr_stub_12(void);
extern void intr_stub_13(void);
extern void intr_stub_14(void);
extern void intr_stub_15(void);
extern void intr_stub_16(void);
extern void intr_stub_17(void);
extern void intr_stub_18(void);

extern void intr_stub_32(void);
extern void intr_stub_33(void);
extern void intr_stub_34(void);
extern void intr_stub_35(void);
extern void intr_stub_36(void);
extern void intr_stub_37(void);
extern void intr_stub_38(void);
extern void intr_stub_39(void);
extern void intr_stub_40(void);
extern void intr_stub_41(void);
extern void intr_stub_42(void);
extern void intr_stub_43(void);
extern void intr_stub_44(void);
extern void intr_stub_45(void);
extern void intr_stub_46(void);
extern void intr_stub_47(void);

extern void intr_stub_48(void);

void idt_set_entry(int i, void (*fn)(), uint16_t selector, uint8_t flags) {
	unsigned long int handler = (unsigned long int) fn;
	idt[i] = handler & 0xffffLL;
	idt[i] |= (selector & 0xffffLL) << 16;
	idt[i] |= (flags & 0xffLL) << 40;
	idt[i] |= ((handler >> 16) & 0xffffLL) << 48;
}

void init_idt() {
	struct {
		unsigned short int limit;
		void* pointer;
	}__attribute__((packed)) idtp = { .limit = IDT_ENTRIES * 8 - 1, .pointer =
			idt, };

	int i = 0;

	for (i = 0; i < IDT_ENTRIES; i++) {
		handler_set[i] = 0;
		irq_rpcs[i] = 0;
	}

	outb(0x20, 0x11); // Initialisierungsbefehl fuer den PIC
	outb(0x21, 0x20); // Interruptnummer fuer IRQ 0
	outb(0x21, 0x04); // An IRQ 2 haengt der Slave
	outb(0x21, 0x01); // ICW 4

	outb(0xa0, 0x11); // Initialisierungsbefehl fuer den PIC
	outb(0xa1, 0x28); // Interruptnummer fuer IRQ 8
	outb(0xa1, 0x02); // An IRQ 2 haengt der Slave
	outb(0xa1, 0x01); // ICW 4

	outb(0x20, 0x0);
	outb(0xa0, 0x0);

	idt_set_entry(0, intr_stub_0, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(1, intr_stub_1, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(2, intr_stub_2, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(3, intr_stub_3, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(4, intr_stub_4, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(5, intr_stub_5, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(6, intr_stub_6, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(7, intr_stub_7, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(8, intr_stub_8, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(9, intr_stub_9, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(10, intr_stub_10, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(11, intr_stub_11, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(12, intr_stub_12, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(13, intr_stub_13, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(14, intr_stub_14, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(15, intr_stub_15, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(16, intr_stub_16, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(17, intr_stub_17, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(18, intr_stub_18, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

	idt_set_entry(32, intr_stub_32, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(33, intr_stub_33, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(34, intr_stub_34, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(35, intr_stub_35, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(36, intr_stub_36, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(37, intr_stub_37, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(38, intr_stub_38, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(39, intr_stub_39, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(40, intr_stub_40, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(41, intr_stub_41, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(42, intr_stub_42, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(43, intr_stub_43, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(44, intr_stub_44, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(45, intr_stub_45, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(46, intr_stub_46, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
	idt_set_entry(47, intr_stub_47, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

	idt_set_entry(48, intr_stub_48, 0x8,
			IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING3 | IDT_FLAG_PRESENT);

	set_gdt_entry(5, (uint32_t) tss, sizeof(tss),
			GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

	asm volatile("ltr %%ax" : : "a" (5 << 3));

	asm volatile("lidt %0" : : "m" (idtp));
	asm volatile("sti");
}

void register_intr_handler(int i, void (*fn)()) {
	handlers[i] = fn;
	handler_set[i] = 1;
}

uint32_t register_irq_rpc(uint32_t irq) {
    if (irq >= 0x20 && irq <= 0x2f) {
        irq_rpcs[irq] = get_current_task()->PID;
        return 1;
    }
    return 0;
}

void disable_irq_rpc(uint32_t irq) {
    if (irq >= 0x20 && irq <= 0x2f) {
        if(irq_rpcs[irq] > 0) {
            irq_rpcs[irq] = -irq_rpcs[irq];
        }
    }
}

void enable_irq_rpc(uint32_t irq) {
    if (irq >= 0x20 && irq <= 0x2f) {
        if(irq_rpcs[irq] < 0) {
            irq_rpcs[irq] = -irq_rpcs[irq];
        }
    }
}

struct cpu_state* handle_interrupt(struct cpu_state* cpu) {
	struct cpu_state* new_cpu = cpu;

	if (cpu->intr <= 0x1f) {
		if (!isSchedulingEnabled()) {
			show_cod(cpu, "Kernel PANIC!");
		} else {
			new_cpu = schedule_exception(cpu);
		}
	} else if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
		if (cpu->intr >= 0x28) {
			outb(0xa0, 0x20);
		}

		if (cpu->intr == 0x20) {
			new_cpu = schedule(cpu);
		}
		else
		{
            if (handler_set[cpu->intr]) {
                handlers[cpu->intr]();
            }
            if(irq_rpcs[cpu->intr] > 0) {
                create_rpc_call(irq_rpcs[cpu->intr], RPCT_IRQ, cpu->intr, 0, 0);
                //disable_irq_rpc(cpu->intr); TODO find out if we need to block
            }
		}

        outb(0x20, 0x20);
	} else if (cpu->intr == 0x30) {
		new_cpu = syscall(new_cpu);
	} else {
		show_cod(cpu, "Unknown Interrupt!");
	}

	return new_cpu;
}
