#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"

uint32_t first_pagedir = 0;
uint32_t scheduling_enabled = 0;

struct cpu_state* current_pdir_cpu = (void*) STATIC_ALLOC_VADDR + 4096 - sizeof(struct cpu_state);
uint32_t* next_pagedir = (void*) STATIC_ALLOC_VADDR + 0x2000;

void enable_scheduling(void) {
  scheduling_enabled = 1;
}

uint32_t init_task(uint32_t task_pagedir, void* entry)
{
  uint32_t old_fpd = first_pagedir;
  uint32_t old_pagedir = vmm_get_current_pagedir();
  uint32_t task_next_pagedir = 0;    
  
  if(first_pagedir == 0) {
    first_pagedir = task_pagedir;
  }
  else
  {
    task_next_pagedir = first_pagedir;
    first_pagedir = task_pagedir;
  }
  
  vmm_activate_pagedir(task_pagedir);
  
                        vmm_alloc_static(0x0000, 0);
  uint8_t* user_stack = vmm_alloc_static(0x1000, PT_PUBLIC);
                        vmm_alloc_static(0x2000, 0);
  
  *next_pagedir = task_next_pagedir;

  struct cpu_state new_state = {
      .eax = 0,
      .ebx = 0,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0,
      .ebp = 0,
      .esp = (uint32_t) user_stack + 4096,
      .eip = (uint32_t) entry,
      
      /* Ring-3-Segmentregister */
      .cs  = 0x18 | 0x03,
      .ss  = 0x20 | 0x03,
      
      .eflags = 0x202,
  };
      
  *current_pdir_cpu = new_state;
  
  vmm_activate_pagedir(old_pagedir);
  
  if(old_fpd == 0) {
    vmm_alloc_static(0x0000, 0);
    vmm_alloc_static(0x1000, PT_PUBLIC);
    vmm_alloc_static(0x2000, 0);
    *next_pagedir = 0;
  }
  
  return task_pagedir;
}

struct cpu_state* schedule(struct cpu_state* cpu)
{
  uint32_t newCPU = 0;
  
  if(first_pagedir != 0 && scheduling_enabled) {  
    uint32_t next = *next_pagedir;
    if(next == 0) next = first_pagedir;
    
    newCPU = 1;            
    vmm_activate_pagedir(next);
  }
  
  if(newCPU != 0) return current_pdir_cpu;
  return cpu;
}

