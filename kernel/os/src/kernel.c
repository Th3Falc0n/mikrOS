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
  uint32_t forkpdir;

  switch (cpu->eax) {
    case 1: /* exit */
      cpu = terminate_current(cpu);
      break;

    case 2: /* fork */
      forkpdir = vmm_fork_current();
      cpu->eax = init_task(forkpdir, 0);
      clone_task_state(forkpdir);
      break;
      
    case 201: /* putc */
      kprintf("%c", cpu->ebx);
      break;
  }

  return cpu;
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
	uint32_t kernel_init_pdir = vmm_init();
	
  map_address_active((uint32_t) mb_info, (uint32_t) mb_info, 0);
  map_address_active((uint32_t) mb_info->mi_mods_addr, (uint32_t) mb_info->mi_mods_addr, 0);
	
  if(mb_info->mi_flags & MULTIBOOT_INFO_HAS_MODS) {
	  for(uint32_t i = 0; i < mb_info->mi_mods_count; i++) {
	    kprintf("Loading mod at %x \n", mb_info->mi_mods_addr[i].start);
	    
	    uint32_t elf_mod_pdir = vmm_create_pagedir();
	    void* elf_mod_entry = 0;
	    
	    vmm_activate_pagedir(elf_mod_pdir);
	    
	    vmm_map_range(mb_info->mi_mods_addr[i].start, mb_info->mi_mods_addr[i].start, mb_info->mi_mods_addr[i].end - mb_info->mi_mods_addr[i].start, 0);
      
      struct elf_header* header = mb_info->mi_mods_addr[i].start;
      struct elf_program_header* ph;

      /* Ist es ueberhaupt eine ELF-Datei? */
      if (header->magic != ELF_MAGIC) {
        kprintf("Invalid ELF-Magic!\n");
        continue;
      }
      
      elf_mod_entry = (void*)( header->entry );
      
      ph = (struct elf_program_header*) (((char*) header) + header->ph_offset);
      for (uint32_t n = 0; n < header->ph_entry_count; n++, ph++) {
        void* dest = (void*) ph->virt_addr;
        void* src = ((char*) header) + ph->offset;
 
        /* Nur Program Header vom Typ LOAD laden */
        if (ph->type != 1) {
            continue;
        }
 
        for(uint32_t offset = 0; offset < ph->mem_size; offset += 0x1000) {
          vmm_alloc_addr(dest + offset, 0);
        }
        
        memcpy(dest, src, ph->file_size);
      }
      
      vmm_activate_pagedir(kernel_init_pdir);
      
      init_task(elf_mod_pdir, elf_mod_entry);
	  }
	
	  enable_scheduling();
	}
	else
	{
	  kprintf("No Modules loadable. Microkernel shutting down.\nThank you for using this pointless version of mikrOS\n");
	}

	while(1) {  }
}