#include "vmm.h"
#include "pmm.h"
#include "console.h"

#define VIRT_PDIR 0xFFFFF000
#define ACT_CONTEXT 0xFF000000

static uint8_t use_phys_addr = 1;

struct vmm_context* vmm_create_context(void)
{
  struct vmm_context* context = pmm_alloc();
  int i;


  context->pagedir = pmm_alloc();
  for (i = 0; i < 1023; i++) {
    context->pagedir[i] = 0;
  }
  context->pagedir[1023] = context->pagedir;
  
  context->vpagedir = VIRT_PDIR;

  vmm_map_page(context, ACT_CONTEXT, context, PTE_PRESENT | PTE_WRITE);
  
  return context;
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys, uint32_t flags)
{
  uint32_t page_index = virt / 0x1000;
  uint32_t pd_index = page_index / 1024;
  uint32_t pt_index = page_index % 1024;
  
  uint32_t* page_dir;
  
  if(use_phys_addr) {
    page_dir = context->pagedir;
  }
  else
  {
    page_dir = context->vpagedir;
  }

  uint32_t* page_table;
  int i;

  /* Wir brauchen 4k-Alignment */
  if ((virt & 0xFFF) || (phys & 0xFFF)) {
			kprintf("map err %x to %x\n", phys, virt);
      return -1;
  }

  if(use_phys_addr) {
    if (page_dir[pd_index] & PTE_PRESENT) {
      page_table = page_dir[pd_index];
    } else {
      page_dir[pd_index] = (uint32_t) pmm_alloc() | PTE_PRESENT | PTE_WRITE;
      
      page_table = page_dir[pd_index];

      for (i = 0; i < 1024; i++) {
          page_table[i] = 0;
      }
    }
  }
  else
  {
    if (page_dir[pd_index] & PTE_PRESENT) {
      page_table = (uint32_t*) (0xFFC00000 + pd_index * 4096);
    } else {
      page_dir[pd_index] = (uint32_t) pmm_alloc() | PTE_PRESENT | PTE_WRITE;
      
      page_table = (uint32_t*) (0xFFC00000  + pd_index * 4096);

      for (i = 0; i < 1024; i++) {
          page_table[i] = 0;
      }
    }
  }

  /* Neues Mapping in the Page Table eintragen */
  page_table[pt_index] = phys | flags;
  asm volatile("invlpg %0" : : "m" (*(char*)virt));

	kprintf("mapped %x to %x\nPT:%x PTI:%x\n", phys, virt, page_table, &page_table[pt_index]);

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
		vmm_map_page(context, (uintptr_t) addr, (uintptr_t) addr, PTE_PRESENT | PTE_WRITE);
		addr += 0x1000;
	}

	addr = 0xB8000;
	while (addr < 0xBFFFF) {
		vmm_map_page(context, (uintptr_t) addr, (uintptr_t) addr, PTE_PRESENT | PTE_WRITE);
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
 
static struct vmm_context* kernel_context;
 
struct vmm_context* vmm_init(struct multiboot_info* mb_info)
{
  kernel_context = vmm_create_context();

	vmm_map_kernel(kernel_context);
	
  struct multiboot_module* modules = mb_info->mi_mods_addr;
  int i;

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

	vmm_set_alloc_offset(kernel_context, 0x200000);
 
  vmm_activate_context(kernel_context);
  
  uint32_t cr0;
  
  while(1) { }
  
  use_phys_addr = 0;

  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= (1 << 31);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));

	return kernel_context;
}
