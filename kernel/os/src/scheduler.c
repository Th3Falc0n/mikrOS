#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"
#include "catofdeath.h"

uint32_t first_pagedir = 0;
uint32_t sched_enabled = 0;

struct cpu_state* current_pdir_cpu = (void*) STATIC_ALLOC_VADDR + 0x1000 - sizeof(struct cpu_state);
uint8_t* user_stack                = (void*) STATIC_ALLOC_VADDR + 0x1000;
uint32_t* next_pagedir             = (void*) STATIC_ALLOC_VADDR + 0x2000;
uint32_t* prev_pagedir             = (void*) STATIC_ALLOC_VADDR + 0x2004;

void enable_scheduling(void) {
  sched_enabled = 1;
}

uint32_t scheduling_enabled(void) {
  return sched_enabled;
}

struct cpu_state* schedule_exception(struct cpu_state* cpu) {
  if(vmm_get_current_pagedir() == first_pagedir && *next_pagedir == 0) {
    //Only one process is running, which just crashed. Stop system.
    show_cod(cpu, "Last task crashed. Terminating kernel...");
  }
  else
  {
    //Potential security leaks available in following code.
    setclr(0x04);
    kprintf("\n Terminated task due to exception %x:%x \n", cpu->intr, cpu->error);
    kprintf("\n");
    show_dump(cpu);
    setclr(0x07);
    show_cod(cpu, "Task crashed. Terminating kernel...");
    
    return terminate_current(cpu);
  }
  return cpu;
}

struct cpu_state* terminate_current(struct cpu_state* cpu) {
  uint32_t next = *next_pagedir;
  uint32_t prev = *prev_pagedir;
  
  //TODO: free resources here
  
  if(vmm_get_current_pagedir() == first_pagedir) {
    first_pagedir = next;
  }
  
  if(next != 0) {
    vmm_activate_pagedir(next);
    *prev_pagedir = prev;
  }

  if(prev != 0) {
    vmm_activate_pagedir(prev);
    *next_pagedir = next;
  }
  

  return schedule(cpu);
}

void clone_task_state(uint32_t task_pagedir) {
  uint32_t old_pagedir = vmm_get_current_pagedir();
  
  uint32_t pdir_seg_paddr = 0;
  uint32_t stack_seg_paddr = 0;
  
  vmm_activate_pagedir(task_pagedir);
  
  pdir_seg_paddr = vmm_resolve((void*)((uint32_t)current_pdir_cpu & 0xFFFFF000));
  stack_seg_paddr = vmm_resolve((void*)((uint32_t)user_stack & 0xFFFFF000));
  
  vmm_activate_pagedir(old_pagedir);
  
  void* fvaddr = vmm_alloc(0);
  vmm_free(fvaddr); //trick to find a empty vaddr
  
  map_address_active((uint32_t)fvaddr, pdir_seg_paddr, PT_ALLOCATABLE);
  memcpy(fvaddr, (void*)((uint32_t)current_pdir_cpu & 0xFFFFF000), 4096);
  
  //vmm_resolve(fvaddr);  //uncomment this and it works
  
  map_address_active((uint32_t)fvaddr, stack_seg_paddr, PT_ALLOCATABLE);
  memcpy(fvaddr, (void*)((uint32_t)user_stack & 0xFFFFF000), 4096);
  
  //vmm_resolve(fvaddr);
  
  vmm_unmap(fvaddr);
  
  //kprintf("fvaddr=%x \n", fvaddr);
  //kprintf("cpdcpu=%x \n", (void*)((uint32_t)current_pdir_cpu & 0xFFFFF000));
  kprintf("\npdir_seg_paddr=%x \n stack_seg_paddr=%x halt.", pdir_seg_paddr, stack_seg_paddr);
  
  while(1);
}

uint32_t init_task(uint32_t task_pagedir, void* entry)
{
  kprintf("Starting task at %x", entry);

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
    vmm_activate_pagedir(task_next_pagedir);
    *prev_pagedir = task_pagedir;
  }
  
  vmm_activate_pagedir(task_pagedir);
  
  vmm_alloc_static(0x0000, PT_PUBLIC);
  vmm_alloc_static(0x1000, PT_PUBLIC);
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
    vmm_alloc_static(0x0000, PT_PUBLIC);
    vmm_alloc_static(0x1000, PT_PUBLIC);
    vmm_alloc_static(0x2000, 0);
    *next_pagedir = 0;
  }
  
  return task_pagedir;
}

struct cpu_state* schedule(struct cpu_state* cpu)
{
  uint32_t newCPU = 0;
  
  if(first_pagedir != 0 && sched_enabled) {  
    uint32_t next = *next_pagedir;
    if(next == 0) next = first_pagedir;
    
    //*current_pdir_cpu = *cpu;
    
    newCPU = 1;            
    vmm_activate_pagedir(next);
  }
  
  if(newCPU) return current_pdir_cpu;
  return cpu;
}

