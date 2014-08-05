#ifndef CATOFDEATH_H
#define CATOFDEATH_H

//#define SCREEN_COD

#include "console.h"
#include "kernel.h"

uint32_t in_cod();
void show_cod(struct cpu_state* cpu, char* fstr);
void show_dump(struct cpu_state* cpu);

#endif
