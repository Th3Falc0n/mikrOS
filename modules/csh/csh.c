#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
 
int main(int argc, char* args[])
{
  kprintf("Forking the shit out of this task... \n");
  int pid = fork();
  
  if(pid) {
    kprintf("PARENT: %x \n", pid);
    while(1);
  }
  else
  {
    kprintf("CHILD: %x \n", pid);
    while(1);
  }
  
  return 0;
}
