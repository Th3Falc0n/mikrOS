#include "scheduler.h"
#include "kernel.h"
#include "ports.h"
#include "console.h"
#include "multiboot.h"
#include "pmm.h"
#include "string.h"
#include "elf.h"
#include "vmm.h"

struct cpu_state* syscall(struct cpu_state* cpu)
{
    switch (cpu->eax) {
        case 0: /* putc */
            kprintf("%c", cpu->ebx);
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

void task() {
  kprintf("task");
  
  while(1) { }
}

void kernel_main(struct multiboot_info* mb_info) {	
	vmm_init(mb_info);

	kprintf("abc");

	while(1) {  }
}
