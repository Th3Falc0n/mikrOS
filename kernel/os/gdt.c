#include "stdint.h"
#include "gdt.h"

#define GDT_ENTRIES 6

uint64_t gdt[GDT_ENTRIES];
 
void load_gdt() {
	struct {
		  uint16_t limit;
		  void* pointer;
	} __attribute__((packed)) gdtp = {
		  .limit = GDT_ENTRIES * 8 - 1,
		  .pointer = gdt,
	};
	asm volatile("lgdt %0" : : "m" (gdtp));

	asm ("mov $0x10, %ax");
	asm ("mov %ax, %ds");
	asm ("mov %ax, %es");
	asm ("mov %ax, %fs");
	asm ("mov %ax, %gs");
	asm ("mov %ax, %ss");
	asm ("ljmp $0x8, $.1");
	asm (".1:");
}

void set_gdt_entry(int i, unsigned int base, unsigned int limit, int flags)
{
    gdt[i] = limit & 0xffffLL;
    gdt[i] |= (base & 0xffffffLL) << 16;
    gdt[i] |= (flags & 0xffLL) << 40;
    gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
    gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
    gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}

void init_gdt(void)
{
    set_gdt_entry(0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_gdt_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_gdt_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    set_gdt_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
 
    load_gdt();
}

