#include "vmm.h"
#include "pmm.h"
#include "console.h"

struct vmm_context* vmm_create_context(void)
{
    struct vmm_context* context = pmm_alloc();
    int i;

 
    context->pagedir = pmm_alloc();
    for (i = 0; i < 1024; i++) {
        context->pagedir[i] = 0;
    }

		vmm_map_page(context, context, context, PTE_PRESENT | PTE_WRITE);
		vmm_map_page(context, context->pagedir, context->pagedir, PTE_PRESENT | PTE_WRITE);
 
    return context;
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys, uint32_t flags)
{
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;

    uint32_t* page_table;
    int i;

    /* Wir brauchen 4k-Alignment */
    if ((virt & 0xFFF) || (phys & 0xFFF)) {
				kprintf("map err %x to %x\n", phys, virt);
        return -1;
    }

    /* Page Table heraussuchen bzw. anlegen */
    if (context->pagedir[pd_index] & PTE_PRESENT) {
        /* Page Table ist schon vorhanden */
        page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
    } else {
        /* Neue Page Table muss angelegt werden */
        page_table = pmm_alloc();

				vmm_map_page(context, page_table, page_table, PTE_PRESENT | PTE_WRITE);

        for (i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        context->pagedir[pd_index] =
            (uint32_t) page_table | PTE_PRESENT | PTE_WRITE;
    }
 
    /* Neues Mapping in the Page Table eintragen */
    page_table[pt_index] = phys | flags;
    asm volatile("invlpg %0" : : "m" (*(char*)virt));

		kprintf("mapped %x to %x\n", phys, virt);
 
    return 0;
}
 
void vmm_activate_context(struct vmm_context* context)
{
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
}

void vmm_map_kernel(struct vmm_context* context) {
	extern const void kernel_start;
	extern const void kernel_end;
	 
	uintptr_t addr = (uintptr_t) &kernel_start;
	while (addr < (uintptr_t) &kernel_end) {
		vmm_map_page(context, addr, addr, PTE_PRESENT | PTE_WRITE);
		addr += 0x1000;
	}

	addr = 0xB8000;
	while (addr < 0xC0000) {
		vmm_map_page(context, addr, addr, PTE_PRESENT | PTE_WRITE);
		addr += 0x1000;
	}
}

void vmm_set_alloc_offset(struct vmm_context* context, uintptr_t offset) {
	context->alloc_offset = offset;
}

void* vmm_alloc(struct vmm_context* context, uint16_t count) {
	uintptr_t ptr = context->alloc_offset;
	
	while(count--) {
		vmm_map_page(context, context->alloc_offset, (uintptr_t) pmm_alloc(), PTE_PRESENT | PTE_WRITE | PTE_USER);
		context->alloc_offset += 0x1000;
	}

	return (void*)(ptr);
}
 
/*
 * Dieser Speicherkontext wird nur waehrend der Initialisierung verwendet.
 * Spaeter laeuft der Kernel immer im Kontext des aktuellen Prozesses.
 */
static struct vmm_context* kernel_context;
 
struct vmm_context* vmm_init(struct multiboot_info* mb_info)
{
  uint32_t cr0;
  int i;

  kernel_context = vmm_create_context();

  struct multiboot_module* modules = mb_info->mi_mods_addr;

	vmm_map_kernel(kernel_context);

  vmm_map_page(kernel_context, (uintptr_t) mb_info, (uintptr_t) mb_info, PTE_PRESENT | PTE_WRITE);
  vmm_map_page(kernel_context, (uintptr_t) modules, (uintptr_t) modules, PTE_PRESENT | PTE_WRITE);

	uintptr_t addr;
  for (i = 0; i < mb_info->mi_mods_count; i++) {
      addr = (uintptr_t)modules[i].start;
      while (addr < (uintptr_t)modules[i].end) {
 				  vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr, PTE_PRESENT | PTE_WRITE);
          addr += 0x1000;
      }
  }

	vmm_set_alloc_offset(kernel_context, 0x0);
 
  vmm_activate_context(kernel_context);

  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= (1 << 31);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));

	return kernel_context;
}
