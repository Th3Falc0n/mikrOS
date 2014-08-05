#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
#include "stdlib.h"

static void fexec(char* path, char* args[]) {
    uint32_t pid = fork();

    if(pid == 0) {
        exec(path, args);

        while(1);
    }
}

static void waitResp(HANDLE cntrl) {
    char resp = 0;

    while(resp != 'K') {
        fread(cntrl, &resp, sizeof(char));
    }
}

int main(int argc, char* args[])
{
    printf("[ibin/init] Init process started... :) Thats so good!\n");
    printf("[ibin/init] Switching into TTY to VGA mode.\nIf you see this something probably went wrong.\n");

    HANDLE cntrl = fmkfifo("/var/cntrl/init");

    fexec("/ibin/ttytovga", 0);
    waitResp(cntrl);

    setstdout("/dev/tty0");
    setstderr("/dev/tty0");

    printf("[init] now working on tty0\n");

    char in = 0;

    print_memstat();

    for(uint32_t i = 0; i < 0xFFFF; i++) {

    }

    char* test = (void*) 0x0;
    *test = 5;

    return 0;
}
