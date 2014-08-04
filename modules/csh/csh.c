#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
 
int main(int argc, char* args[])
{
  printf("Forking the shit out of this task... \n");
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
