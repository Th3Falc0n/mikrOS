#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "process.h"
#include "string.h"
 
int main(int argc, char* args[])
{
    uint32_t index = 0;
    char child[512];

    char* path = args[1];

    if(!path) {
    	path = malloc(512);
    	getExecPath(path);
    }

    while(getPathChild(index++, child, path)) {

    	if(child[strlen(child)-1] == '/') {
    	    putchar(0x11);
    	    putchar(0x31);
    	}

		printf("%s\n", child);

	    putchar(0x11);
	    putchar(0x07);
    }

    free(path);

    return 0;
}
