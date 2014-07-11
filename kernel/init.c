#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "os/drivers/drvinit.h"
#include "kernel.h"
#include "multiboot.h"
#include "pmm.h"

#define _VERSION "alphaKernelDev 1.01"

void init(struct multiboot_info* mb_info) {
	clrscr();
	pmm_init(mb_info);

	kprintf("Welcome to mikrOS (version '%s')!\n", _VERSION);
	kprintf("(C) Copyright 2012-2014 Fabian Sachara.  All Rights Reserved.\n");

	kprintf("Initializing GDT...\n");

	init_gdt();

	kprintf("Initializing IDT...\n");

	init_idt();

	kprintf("Initializing Drivers...\n");

	init_drivers();

	kprintf("Initializing Kernel...\n");

	pmm_print_stats();

	kernel_main(mb_info);

	return;
}
