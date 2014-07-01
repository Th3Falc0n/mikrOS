#include "syscall.h"

void syscall(struct regstate* state) {
  asm("mov %0, %%ebx" : : "r" (state->ebx));
  asm("mov %0, %%ecx" : : "r" (state->ecx));
  asm("mov %0, %%edx" : : "r" (state->edx));
  asm("mov %0, %%esi" : : "r" (state->esi));
  asm("mov %0, %%edi" : : "r" (state->edi));
  
  asm("int $0x30" : : "a" (state->eax));
  
  asm("mov %%eax, %0" : "=r" (state->eax));
  asm("mov %%ebx, %0" : "=r" (state->ebx));
  asm("mov %%ecx, %0" : "=r" (state->ecx));
  asm("mov %%edx, %0" : "=r" (state->edx));
  asm("mov %%esi, %0" : "=r" (state->esi));
  asm("mov %%edi, %0" : "=r" (state->edi));
}
