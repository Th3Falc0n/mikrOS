#include "stdint.h"
#include "stdio.h"
#include "process.h"
 
void _start(void)
{
  int pid = fork();

  int i;
	for (i = 0; 1; i = (i+1) % 5) {
	  if(pid) {
	    printf("PARENT: %d\n", pid);
	  }
	  else
	  {
	    printf("CHILD:  %d\n", pid);
	  }
	}

  while(1);
}
