#include "process.h"

extern int main();

void _start() {
  exit(main());
}

void exit(int returncode) {
  struct regstate state = {
    .eax = 1,
    .ebx = returncode,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);
  
  while(1);
}

uint32_t fork(void) {
  struct regstate state = {
    .eax = 2,
    .ebx = 0,
    .ecx = 0,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };
  
  syscall(&state);  
  
  return state.eax;
}
