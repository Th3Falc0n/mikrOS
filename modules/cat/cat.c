#include "stdint.h"
#include "stdio.h"
#include "process.h"
 
int main(int argc, char* args[])
{
    if(argc < 2) {
        //printf("cat: No file specified\n");

        HANDLE in = fopen("/dev/urandom", FM_READ);

        char c;

        while(1) {
            fread(in, &c, sizeof(char));
            putchar(c);
        }
    }
    return 0;
}
