#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
 
int main(void)
{
    printf("[ibin/init] Init process started... :) Thats so good!\n");

    uint32_t pid = fork();

    if(pid == 0) {
        exec("/ibin/csh", 0);
    }

    return 0;
}
