#include "vmm.h"

extern const void kernel_start;
extern const void kernel_end;

uint32_t* active_pagetables        = (void*)(ACTIVE_PAGETBL_VADDR);
struct vmm_context* active_context = (void*)(ACTIVE_CONTEXT_VADDR);

uint32_t active_pagedir = 0;

uint32_t vmm_get_current_pagedir(void) {
  return active_pagedir;
}

//TODO: join fork_current and create_pagedir together

uint32_t vmm_fork_current(void)
{
  uint32_t phys_context = 0;
  struct vmm_context* context = vmm_alloc(&phys_context);
  
  uint32_t phys_pagedir = 0;
  context->pagedir = vmm_alloc(&phys_pagedir);
  
  uint32_t paddr, i, i2, pagedir_ptr[1024];
    
  for(i = 0; i < 1024; i++) {
    pagedir_ptr[i] = (uint32_t) vmm_alloc(&paddr);
    context->pagedir[i] = paddr | PD_PRESENT | PD_WRITE | PD_PUBLIC;
    
    for(i2 = 0; i2 < 1024; i2++) {
      uint32_t vaddr = (i << 24) + (i2 << 12);
    
      ((uint32_t*)(pagedir_ptr[i]))[i2] = (vaddr > USERSPACE_BOTTOM) ? PT_ALLOCATABLE : 0;
    }
  }
  
  for(i = 0; i < 1024; i++) {        
    paddr = context->pagedir[i] & 0xFFFFF000;
    map_address_context(pagedir_ptr, (uint32_t)active_pagetables + i * 0x1000, paddr, 0);
  }
  
  map_address_context(pagedir_ptr, (uint32_t) active_context, phys_context, 0);
  
  for(i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
    map_address_context(pagedir_ptr, i, i, 0);
  }
  
  for(i = USERSPACE_BOTTOM; i < 0xFFFFF000; i += 0x1000) {
    if((active_pagetables[i >> 12] & (PT_ALLOCATABLE | PT_PRESENT)) == (PT_ALLOCATABLE | PT_PRESENT)) {
      void* newp = vmm_alloc(&paddr);
      
      kprintf("copying program %x \n", i);
      
      memcpy(newp, (void*)i, 0x1000);
      map_address_context(pagedir_ptr, i, paddr, PT_PUBLIC | PT_ALLOCATABLE);
      
      vmm_unmap(newp);
    }
  }
  
  //Following code has a memory leak. A context must free its own resources on destruction.
  
  for(i = 0; i < 1024; i++) {       
    vmm_unmap((void*)pagedir_ptr[i]); 
  }
  
  vmm_unmap(pagedir_ptr);
  
  uint32_t* pd_ptr = context->pagedir;
  context->pagedir = (uint32_t*) phys_pagedir;
  
  vmm_unmap(pd_ptr);
  vmm_unmap(context);
  
  return phys_pagedir;
}

uint32_t vmm_create_pagedir()
{
  uint32_t phys_context = 0;
  struct vmm_context* context = vmm_alloc(&phys_context);
  
  uint32_t phys_pagedir = 0;
  context->pagedir = vmm_alloc(&phys_pagedir);
  
  uint32_t paddr, i, i2, pagedir_ptr[1024];
    
  for(i = 0; i < 1024; i++) {
    pagedir_ptr[i] = (uint32_t) vmm_alloc(&paddr);
    context->pagedir[i] = paddr | PD_PRESENT | PD_WRITE | PD_PUBLIC;
    
    for(i2 = 0; i2 < 1024; i2++) {
      uint32_t vaddr = (i << 24) + (i2 << 12);
    
      ((uint32_t*)(pagedir_ptr[i]))[i2] = (vaddr > USERSPACE_BOTTOM) ? PT_ALLOCATABLE : 0;
    }
  }
  
  for(i = 0; i < 1024; i++) {        
    paddr = context->pagedir[i] & 0xFFFFF000;
    map_address_context(pagedir_ptr, (uint32_t)active_pagetables + i * 0x1000, paddr, 0);
  }
  
  map_address_context(pagedir_ptr, (uint32_t) active_context, phys_context, 0);
  
  for(i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
    map_address_context(pagedir_ptr, i, i, 0);
  }
  
  //Following code has a memory leak. A context must free its own resources on destruction.
  
  for(i = 0; i < 1024; i++) {       
    vmm_unmap((void*)pagedir_ptr[i]); 
  }
  
  vmm_unmap(pagedir_ptr);
  
  uint32_t* pd_ptr = context->pagedir;
  context->pagedir = (uint32_t*) phys_pagedir;
  
  vmm_unmap(pd_ptr);
  vmm_unmap(context);
  
  return phys_pagedir;
}

uint32_t vmm_resolve(void* vaddr) {
  return active_pagetables[(uint32_t)vaddr >> 12] & 0xFFFFF000;
}

void vmm_map_range(void* vaddr, void* paddr, uint32_t length, uint32_t flags) {
  if((uint32_t)vaddr & 0xFFF) return;
  if((uint32_t)paddr & 0xFFF) return;
  
  for(uint32_t i = 0; i < length; i+= 0x1000) {
    map_address_active((uint32_t)vaddr + i, (uint32_t)paddr + i, flags);
  }
}

void map_address_context(uint32_t* pagedir, uint32_t vaddr, uint32_t paddr, uint32_t flags) {
  uint32_t pd_entry =  vaddr >> 22;
  uint32_t pt_entry = (vaddr >> 12) % 1024;
  
  ((uint32_t*)(pagedir[pd_entry] & 0xFFFFF000))[pt_entry] = (paddr & 0xFFFFF000) | PT_PRESENT | PT_WRITE | (flags & 0xFFF);
}

void map_address_active(uint32_t vaddr, uint32_t paddr, uint32_t flags) {  
  active_pagetables[vaddr >> 12] = (paddr & 0xFFFFF000) | PT_PRESENT | PT_WRITE | (flags & 0xFFF);
  asm volatile("invlpg %0" : : "m" (vaddr));
}

void vmm_free(void* p_vaddr) {
  uint32_t vaddr = (uint32_t)p_vaddr;

  if((active_pagetables[vaddr >> 12] & (PT_ALLOCATABLE | PT_PRESENT)) == (PT_ALLOCATABLE | PT_PRESENT)) {
    pmm_free((void*)(active_pagetables[vaddr >> 12] & 0xFFFFF000));
   
    active_pagetables[vaddr >> 12] = PT_ALLOCATABLE;
    asm volatile("invlpg %0" : : "m" (vaddr));
  }
}

void vmm_unmap(void* p_vaddr) { //USE ONLY IF YOU KNOW WHAT YOU DO. POTENTIAL MEMORY LEAK!
  uint32_t vaddr = (uint32_t)p_vaddr;

  if((active_pagetables[vaddr >> 12] & (PT_ALLOCATABLE | PT_PRESENT)) == (PT_ALLOCATABLE | PT_PRESENT)) {
    active_pagetables[vaddr >> 12] = PT_ALLOCATABLE;
    asm volatile("invlpg %0" : : "m" (vaddr));
  }
}


void* vmm_alloc(uint32_t* retpaddr) {
  void* vaddr = 0;

  for(uint32_t i = USERSPACE_BOTTOM; i <= 0xFFFFF000; i += 0x1000) {
    if((active_pagetables[i >> 12] & (PT_ALLOCATABLE | PT_PRESENT)) == PT_ALLOCATABLE) {
      vaddr = (void*) i;
      break;
    }
  }
 
  return vmm_alloc_addr(vaddr, retpaddr);  
}

void* vmm_alloc_addr(void* reqvaddr, uint32_t* retpaddr) {
  if((active_pagetables[(uint32_t)reqvaddr >> 12] & (PT_ALLOCATABLE | PT_PRESENT)) != PT_ALLOCATABLE) return 0;

  uint32_t paddr = (uint32_t) pmm_alloc();
  map_address_active((uint32_t)reqvaddr, paddr, PT_PUBLIC | PT_ALLOCATABLE);
  
  if(retpaddr != 0) *retpaddr = paddr;
  
  return reqvaddr;
}

//TODO: VMM_ALLOC_STATIC will override already allocated memory

void* vmm_alloc_static(uint32_t offset, uint32_t flags) {
  if(offset < 0x100000) {
    if((offset & 0xFFFFF000) != offset) {
      kprintf("Tried to allocate not 4K-aligned static vaddr. \n");
    }
    uint32_t vaddr = offset + STATIC_ALLOC_VADDR;
    uint32_t paddr = (uint32_t) pmm_alloc();
    
    map_address_active(vaddr, paddr, flags & 0xFFF);
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
  
  context->pagedir = pmm_alloc();
  
  for(i = 0; i < 1024; i++) {
    context->pagedir[i] = ((int)pmm_alloc()) | PD_PRESENT | PD_WRITE | PD_PUBLIC;
        
    for(i2 = 0; i2 < 1024; i2++) {
      uint32_t vaddr = (i << 24) + (i2 << 12);
      
      ((uint32_t*)(context->pagedir[i] & 0xFFFFF000))[i2] = (vaddr > USERSPACE_BOTTOM) ? PT_ALLOCATABLE : 0;
    }
  }
    
  for(i = 0; i < 1024; i++) {    
    uint32_t paddr = context->pagedir[i] & 0xFFFFF000;
    map_address_context(context->pagedir, (uint32_t)active_pagetables + i * 0x1000, paddr, 0);
  }
  
  map_address_context(context->pagedir, (uint32_t) active_context, (uint32_t) context, 0);
  
  for(i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
    map_address_context(context->pagedir, i, i, PT_PUBLIC); //TODO: no PT_PUBLIC flag here!... only for task testing
  }
  
  //END CREATE CONTEXT ********************************************************
  
  vmm_activate_pagedir((uint32_t) context->pagedir);

  uint32_t cr0;

  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= (1 << 31);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
  
  return (uint32_t) active_context->pagedir;
}
