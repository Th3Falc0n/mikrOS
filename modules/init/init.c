#include "stdint.h"
#include "stdio.h"
#include "vmmcall.h"
#include "process.h"
#include "stdlib.h"

static void fexec(char* path, char* args[]) {
    uint32_t pid = fork();

    //TODO: fix args (will be overriden in new vmm_context, cause of rewriting program) :)

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

    print_memstat();

    fexec("/ibin/urnd_prov", 0);
    waitResp(cntrl);

    /*HANDLE test = fopen("/dev/urandom", FM_READ);

    char* c = malloc(513);
    c[512] = '\0';

    while(1) {
        fread(test, c, 512);

        puts(c);
    }*/

    printf("[ibin/init] Finished. Starting /etc/init/post_ramfs");

    while(1);

    return 0;
}
