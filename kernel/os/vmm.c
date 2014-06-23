#include "vmm.h"
#include "pmm.h"
#include "console.h"

#define PD_EXIST   (1 << 0)
#define PD_WRITE   (1 << 1)
#define PD_PUBLIC  (1 << 2)
#define PD_WTCACHE (1 << 3)
#define PD_NOCACHE (1 << 4)
#define PD_USED    (1 << 5)
#define PD_USER0   (1 << 9)
#define PD_USER1   (1 << 10)
#define PD_USER2   (1 << 11)

#define PT_EXIST   (1 << 0)
#define PT_WRITE   (1 << 1)
#define PT_PUBLIC  (1 << 2)
#define PT_WTCACHE (1 << 3)
#define PT_NOCACHE (1 << 4)
#define PT_USED    (1 << 5)
#define PT_WRITTEN (1 << 6)
#define PT_USER0   (1 << 9)
#define PT_USER1   (1 << 10)
#define PT_USER2   (1 << 11)

#define ACTIVE_CONTEXT_VADDR 0x200000
#define PROGRAM_ZERO 0x1000000

extern const kernel_end;

struct vmm_context* vmm_create_context(void)
{
  struct vmm_context* context = pmm_alloc();
  int i;

  /* Page Directory anlegen und mit Nullen initialisieren */
  context->pagedir = pmm_alloc();
  for (i = 0; i < 1024; i++) {
      //context->pagedir[i] = pmm_alloc() | PD_EXIST | PD_WRITE | PD_PUBLIC;
  }

  return context;
}

void map_address_context(struct vmm_context* context, uint32_t vaddr, uint32_t paddr) {
  uint32_t pd_entry =  vaddr >> 22;
  uint32_t pt_entry = (vaddr >> 12) % 1024;
  
  ((uint32_t*)(context->pagedir[pd_entry] & 0xFFFFF000))[pt_entry] = (paddr & 0xFFFFF000) | PT_EXIST | PT_WRITE | PT_PUBLIC;
}

void map_address_active(uint32_t vaddr, uint32_t paddr) {  
  ((uint32_t*)(ACTIVE_CONTEXT_VADDR))[vaddr >> 12] = (paddr & 0xFFFFF000) | PT_EXIST | PT_WRITE | PT_PUBLIC;
}

void map_context_to_ctvaddr(struct vmm_context* context) {
  for(i = 0; i < 1024; i++) {
    for(i2 = 0; i2 < 1024; i2++) {
      map_address_context(context, ACTIVE_CONTEXT_VADDR + i * 0x1000 + i2 * 0x4, &((uint32_t*)(context->pagedir[i] & 0xFFFFF000))[i2]);
    }
  }
}

void vmm_init(struct multiboot_info* mb_info)
{
  struct vmm_context* context = pmm_alloc();
  int i, i2;
  
  context->pagedir = pmm_alloc();
  for(i = 0; i < 1024; i++) {
    context->pagedir[i] = ((int)pmm_alloc()) | PD_EXIST | PD_WRITE | PD_PUBLIC;
    for(i2 = 0; i2 < 1024; i2++) {
      ((uint32_t*)(context->pagedir[i] & 0xFFFFF000))[i2] = 0;
    }
  }
  
  for(i = 0x0000; i < &kernel_end; i += 0x1000) {
    map_address_context(context, i, i);
  }
  
  map_context_to_ctvaddr(context);
  
  asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));

  uint32_t cr0;

  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= (1 << 31);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
  
  uint32_t* vptr = 0x8F8000;
  uint32_t paddr = pmm_alloc();
  
  map_address_active(vptr, paddr);
  
  *vptr = 0x12345678;
  
  kprintf("%x", *vptr);
}
