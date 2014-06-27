#include "scheduler.h"
#include "kernel.h"
#include "ports.h"
#include "console.h"
#include "multiboot.h"
#include "pmm.h"
#include "stdlib.h"
#include "elf.h"
#include "vmm.h"
#include "idt.h"

struct cpu_state* syscall(struct cpu_state* cpu)
{
  switch (cpu->eax) {
    case 0: /* free_cpu */
      cpu = schedule(cpu);
      break;
  }

  return cpu;
}

void init_elf(void* image)
{
    /*
     * FIXME Wir muessen eigentlich die Laenge vom Image pruefen, damit wir bei
     * korrupten ELF-Dateien nicht ueber das Dateiende hinauslesen.
     */
 
    struct elf_header* header = image;
    struct elf_program_header* ph;
    int i;
 
    /* Ist es ueberhaupt eine ELF-Datei? */
    if (header->magic != ELF_MAGIC) {
        kprintf("Keine gueltige ELF-Magic!\n");
        return;
    }
 
    ph = (struct elf_program_header*) (((char*) image) + header->ph_offset);
    for (i = 0; i < header->ph_entry_count; i++, ph++) {
        void* dest = (void*) ph->virt_addr;
        void* src = ((char*) image) + ph->offset;
 
        /* Nur Program Header vom Typ LOAD laden */
        if (ph->type != 1) {
            continue;
        }
 
        memset(dest, 0, ph->mem_size);
        memcpy(dest, src, ph->file_size);
    }
 
    //init_task((void*) header->entry);
}

void task2() {
  uint32_t n = 0;
  
  kprintf("2222: %x \n", n);

  while(1) { 
    n++;
  }
}

void task1() {
  uint32_t n = 0;
  
  kprintf("1111: %x \n", n);
  while(1) { 
    n++;
  }
}

void kernel_main(struct multiboot_info* mb_info) {	
	vmm_init(mb_info);
	
	void* alloc = vmm_alloc(0);
	kprintf("alloc 1: %x \n", alloc);
	
	alloc = vmm_alloc(0);
	kprintf("alloc 2: %x \n", alloc);
	
	vmm_free(alloc);
	alloc = vmm_alloc(0);
	kprintf("alloc 3: %x \n", alloc);
	
	init_task(vmm_create_pagedir(mb_info), task2);
	init_task(vmm_create_pagedir(mb_info), task1);
	  
	enable_scheduling();
	
	//uint32_t i;
  /*if(mb_info->mi_flags & MULTIBOOT_INFO_HAS_MODS) {
	  for(i = 0; i < mb_info->mi_mods_count; i++) {
	    init_elf((void*) mb_info->mi_mods_addr[i].start);
	    kprintf("MODINIT: ");
	    //kprintf(mb_info->mi_mods_addr[i].cmdline);
	    kprintf("\n");
	  }
	
	  enable_scheduling();
	}
	else
	{
	  kprintf("No Modules loadable. Microkernel shutting down.\nThank you for using this pointless version of mikrOS\n");
	}*/

	while(1) {  }
}
