#include "catofdeath.h"

void show_cod(struct cpu_state* cpu) {
  #ifdef SCREEN_COD
  clrscr();

  kprintf ("        (`. \n");
  kprintf ("         ) ) \n");
  kprintf ("        ( ( \n");
  kprintf ("         \\ \\ \n");
  kprintf ("          \\ \\ \n");
  kprintf ("        .-'  `-. \n");
  kprintf ("       /        `. \n");
  kprintf ("      (      )    `-._ ,    _ \n");
  kprintf ("       )   ,'         (.\\--'( \n");
  kprintf ("       \\  (         ) /      \\ \n");
  kprintf ("        \\  \\_(     / (    <6 (6 \n");
  kprintf ("         \\_)))\\   (   `._  .:Y)__ \n");
  kprintf ("          '''  \\   `-._.'`---^_))) \n");
  kprintf ("                `-._ )))       ``` \n");
  kprintf ("                     ```            \n");
  #endif
  
  kprintf("\nException I:%d E:%x, Kernel halt!\n", cpu->intr, cpu->error);

  kprintf("EAX: %x EBX: %x ECX: %x EDX: %x\n", cpu->eax, cpu->ebx, cpu->ecx, cpu->edx);
  kprintf("ESI: %x EDI: %x EBP: %x EIP: %x\n", cpu->esi, cpu->edi, cpu->ebp, cpu->eip);
  kprintf("CS: %x EFLAGS: %x ESP: %x SS: %x\n", cpu->cs, cpu->eflags, cpu->esp, cpu->ss);

  uint32_t cr2 = 0;

  asm volatile("mov %%cr2, %0" : "=r" (cr2));

  kprintf("CR2: %x \n", cr2);

  while(1) {
    asm volatile("cli; hlt");
  }
}
