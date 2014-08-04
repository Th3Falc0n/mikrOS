#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
#include "stdlib.h"

static void fexec(char* path) {
    uint32_t pid = fork();

    if(pid == 0) {
        exec(path, 0);
    }
}

int main(int argc, char* args[])
{
    kprintf("[ibin/init] Init process started... :) Thats so good!\n");

    HANDLE file = fopen("/cfg/test", FM_READ);

    char* string = malloc(16);
    fread(file, string, 15);
    fclose(file);

    kprintf("[ibin/init] test string: %s\n", string);

    fexec("/ibin/ttytovga");

    return 0;
}
