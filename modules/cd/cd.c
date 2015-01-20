#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
 
int main(int argc, char* args[])
{
    if(args[1] != 0) {
        cd((char*)args[1]);
    }
    else
    {
        printf("cd: usage: \"cd <PATH>\"\n");
    }

    return 0;
}
