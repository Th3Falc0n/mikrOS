#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
 
int main(int argc, char* args[])
{
    if(argc > 0) {
        printf("CSH started with %d arguments (args[0] = %s)\n", argc, args[0]);
    }
    else
    {
        printf("CSH started with 0 arguments\n");
    }

    return 0;
}
