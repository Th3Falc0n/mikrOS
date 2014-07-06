#include "stdint.h"
#include "stdio.h"
#include "process.h"
 
void _start(void)
{
  printf("Forking the shit out of this task... \n\n");
  int pid = fork();
  
  if(pid) {
    printf("PARENT: %x \n", pid);
    while(1);
  }
  else
  {
    printf("CHILD: %x \n", pid);
    while(1);
  }
}
