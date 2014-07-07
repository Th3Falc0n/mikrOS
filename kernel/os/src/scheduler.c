#include "scheduler.h"
#include "kernel.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"
#include "catofdeath.h"

uint32_t sched_enabled = 0;

struct task* first_task   = 0;
struct task* current_task = 0;

int nextPID = 1;

void enable_scheduling(void) {
  sched_enabled = 1;
}

uint32_t scheduling_enabled(void) {
  return sched_enabled;
}

struct cpu_state* schedule_exception(struct cpu_state* cpu) {
  if(current_task == first_task && current_task->next == 0) {
    //Only one process is running, which just crashed. Stop system.
    setclr(0x04);
    kprintf("\n Terminated task (PID=%d) due to exception %x:%x \n", current_task->PID, cpu->intr, cpu->error);
    show_cod(cpu, "Last task crashed. Terminating kernel...");
  }
  else
  {
    //Potential security leaks available in following code.
    setclr(0x04);
    kprintf("\n Terminated task (PID=%d) due to exception %x:%x \n", current_task->PID, cpu->intr, cpu->error);
    kprintf("\n");
    show_dump(cpu);
    setclr(0x07);
    
    return terminate_current(cpu);
  }
  return cpu;
}

struct cpu_state* terminate_current(struct cpu_state* cpu) {
  struct task* next = current_task->next;
  struct task* prev = current_task->prev;
  
  //TODO: free resources here
  
  if(current_task == first_task) {
    first_task = current_task->next;
  }
  
  if(next != 0) {
    next->prev = prev;
  }

  if(prev != 0) {
    prev->next = next;
  }
  
  //TODO: handle if all tasks are closed

  if(next == 0) next = first_task;
    
  current_task = next;       
  vmm_activate_pagedir(current_task->phys_pdir);     
  return current_task->cpu_state;
}

void fork_task_state(struct task* new_task) {      
  new_task->user_stack_bottom = current_task->user_stack_bottom;
  
  memcpy(new_task->cpu_state, current_task->cpu_state, sizeof(struct cpu_state));
  
  new_task->cpu_state->eax = 0;
}

struct task* init_task(uint32_t task_pagedir, void* entry)
{
  struct task* ntask = malloc(sizeof(struct task));
  ntask->cpu_state   = malloc(sizeof(struct cpu_state));
  
  ntask->phys_pdir = task_pagedir;
  ntask->user_stack_bottom = (void*)0xFFFFE000;
  ntask->PID = nextPID++;
  
  ntask->next = (void*)0;
  ntask->prev = (void*)0;
  
  if(first_task == 0) {
    first_task = ntask;
  }
  else
  {
    ntask->next = first_task;
    first_task->prev = ntask;
    first_task = ntask;
  }
  
  uint32_t rest_pdir = vmm_get_current_pagedir();
  vmm_activate_pagedir(task_pagedir);
  
  if(entry != 0) { //entry == 0 means that this will be forked
    vmm_alloc_addr(ntask->user_stack_bottom, 0);
  }

  struct cpu_state nstate = {
    .eax = 0,
    .ebx = 0,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0,
    .ebp = 0,
    .esp = (uint32_t)ntask->user_stack_bottom + 4096,
    .eip = (uint32_t)entry,
    
    /* Ring-3-Segmentregister */
    .cs  = 0x18 | 0x03,
    .ss  = 0x20 | 0x03,
    
    .eflags = 0x200,
  };
  
  memcpy(ntask->cpu_state, &nstate, sizeof(struct cpu_state));
    
  vmm_activate_pagedir(rest_pdir);
  
  return ntask;
}

void save_cpu_state(struct cpu_state* cpu) {
  memcpy(current_task->cpu_state, cpu, sizeof(struct cpu_state));
}

struct cpu_state* schedule(struct cpu_state* cpu)
{
  if(first_task != 0 && sched_enabled) {  
    if(current_task == 0) {
      current_task = first_task;
      vmm_activate_pagedir(current_task->phys_pdir);      
      return current_task->cpu_state;
    }
    
    struct task* next = current_task->next;
    if(next == 0) next = first_task;
  
    save_cpu_state(cpu);
        
    current_task = next;       
    vmm_activate_pagedir(current_task->phys_pdir);     
    return current_task->cpu_state;
  }
  return cpu;
}

