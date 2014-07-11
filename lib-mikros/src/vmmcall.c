#include "vmmcall.h"

void* vmm_alloc_cont (uint32_t cont) {
  struct regstate state = {
    .eax = 203,
    .ebx = cont,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);  
  
  return (void*)state.eax;
}

void vmm_free (void* vaddr) {
  struct regstate state = {
    .eax = 204,
    .ebx = (uint32_t)vaddr,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);  
}

void print_memstat() {
  struct regstate state = {
    .eax = 205,
    .ebx = 0,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);  
}
