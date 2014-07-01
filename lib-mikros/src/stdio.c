#include "stdio.h"

int putc(char c) {
  struct regstate state = {
    .eax = 201,
    .ebx = (uint32_t)c,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);
  
  return state.eax;
}
