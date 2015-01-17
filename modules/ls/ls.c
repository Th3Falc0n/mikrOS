#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
 
int main(int argc, char* args[])
{
    uint32_t index = 0;
    char child[512];


    while(getExecPathChild(index++, child)) {

    	if(child[strlen(child)-1] == '/') {
    	    putchar(0x11);
    	    putchar(0x31);
    	}

		printf("%s\n", child);

	    putchar(0x11);
	    putchar(0x07);
    }

    return 0;
}
