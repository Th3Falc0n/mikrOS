#include "vmm.h"

extern const void kernel_start;
extern const void kernel_end;

struct vmm_context* active_context = (void*)(ACTIVE_CONTEXT_VADDR + 0x400000);

uint32_t active_pagedir = 0;

uint32_t vmm_get_current_pagedir(void) {
  return active_pagedir;
}

uint32_t vmm_create_pagedir(void)
{
  uint32_t phys_context = 0;
  struct vmm_context* context = vmm_alloc(&phys_context);
  
  uint32_t phys_pagedir = 0;
  context->pagedir = vmm_alloc(&phys_pagedir);
  
  uint32_t paddr, i, i2, pagedir_ptr[1024];
  for(i = 0; i < 1024; i++) {
    pagedir_ptr[i] = (uint32_t) vmm_alloc(&paddr);
    context->pagedir[i] = paddr | PD_EXIST | PD_WRITE | PD_PUBLIC;
    
    for(i2 = 0; i2 < 1024; i2++) {
      ((uint32_t*)(pagedir_ptr[i]))[i2] = 0;
    }
  }
  
  for(i = 0; i < 1024; i++) {        
    paddr = context->pagedir[i] & 0xFFFFF000;
    map_address_context(pagedir_ptr, ACTIVE_CONTEXT_VADDR + i * 0x1000, paddr);
  }
  
  map_address_context(pagedir_ptr, (uint32_t) active_context, phys_context);
  
  for(i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
    map_address_context(pagedir_ptr, i, i);
  }
  
  context->pagedir = (uint32_t*) phys_pagedir;
  
  return phys_pagedir;
}

void map_address_context(uint32_t* pagedir, uint32_t vaddr, uint32_t paddr) {
  uint32_t pd_entry =  vaddr >> 22;
  uint32_t pt_entry = (vaddr >> 12) % 1024;
  
  ((uint32_t*)(pagedir[pd_entry] & 0xFFFFF000))[pt_entry] = (paddr & 0xFFFFF000) | PT_EXIST | PT_WRITE | PT_PUBLIC;
}

void map_address_active(uint32_t vaddr, uint32_t paddr) {  
  ((uint32_t*)(ACTIVE_CONTEXT_VADDR))[vaddr >> 12] = (paddr & 0xFFFFF000) | PT_EXIST | PT_WRITE | PT_PUBLIC;
}

//TODO: VMM_ALLOC can't free memory

void* vmm_alloc(uint32_t* retpaddr) {
  void* retaddr = (void*) active_context->alloc_addr_count;
  
  uint32_t paddr = (uint32_t) pmm_alloc();
  map_address_active(active_context->alloc_addr_count, paddr);
  
  if(retpaddr != 0) *retpaddr = paddr;
  active_context->alloc_addr_count += 0x1000;
  
  return retaddr;
}

//TODO: VMM_ALLOC_STATIC will override already allocated memory

void* vmm_alloc_static(uint32_t offset) {
  if(offset < 0x100000) {
    if((offset & 0xFFFFF000) != offset) {
      kprintf("Tried to allocate not 4K-aligned static vaddr. \n");
    }
    uint32_t vaddr = offset + STATIC_ALLOC_VADDR;
    uint32_t paddr = (uint32_t) pmm_alloc();
    map_address_active(vaddr, paddr);
    return (void*) vaddr;
  }
  else
  {
    kprintf("Tried to allocate static vaddr outside reserved range. \n");
    return 0;
  }
}

void vmm_activate_pagedir(uint32_t pdpaddr) {
  active_pagedir = pdpaddr;
  asm volatile("mov %0, %%cr3" : : "r" (pdpaddr));
}

uint32_t vmm_init(void)
{
  //CREATE CONTEXT ************************************************************
  
  struct vmm_context* context = pmm_alloc();
  uint32_t i, i2;
  
  context->alloc_addr_count = USERSPACE_ZERO;
  context->alloc_byte_count = 0;
  
  context->pagedir = pmm_alloc();
  
  for(i = 0; i < 1024; i++) {
    context->pagedir[i] = ((int)pmm_alloc()) | PD_EXIST | PD_WRITE | PD_PUBLIC;
        
    for(i2 = 0; i2 < 1024; i2++) {
      ((uint32_t*)(context->pagedir[i] & 0xFFFFF000))[i2] = 0;
    }
  }
    
  for(i = 0; i < 1024; i++) {    
    uint32_t paddr = context->pagedir[i] & 0xFFFFF000;
    map_address_context(context->pagedir, ACTIVE_CONTEXT_VADDR + i * 0x1000, paddr);
  }
  
  map_address_context(context->pagedir, (uint32_t) active_context, (uint32_t) context);
  
  for(i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
    map_address_context(context->pagedir, i, i);
  }
  
  //END CREATE CONTEXT ********************************************************
  
  vmm_activate_pagedir((uint32_t) context->pagedir);

  uint32_t cr0;

  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= (1 << 31);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
  
  return (uint32_t) active_context->pagedir;
}
