#include "process.h"

extern int main(int argc, char* args[]);

static uint32_t getargs() {
    struct regstate state = {
      .eax = 4,
      .ebx = 0,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
}

void _start() {
  char** args = (char**) getargs();

  int argc = 0;

  if(args != 0) {
      while(args[argc] != 0) {
          argc++;
      }
  }

  exit(main(argc, args));
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


void exec(char* path, char** args) {
  struct regstate state = {
    .eax = 3,
    .ebx = (uint32_t) path,
    .ecx = (uint32_t) args,
    .edx = 0,
    .esi = 0,
    .edi = 0
  };

  syscall(&state);
}
