#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
 
int main(void)
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
  
  return 0;
}
