#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "stdlib.h"

static void waitResp(HANDLE cntrl) {
    char resp = 0;

    while(resp != 'K') {
        fread(cntrl, &resp, sizeof(char));
    }
}

static void hdlrx21(uint32_t irq) {
    printf("IRQ 0x21!\n");
}

int main(int argc, char* args[])
{
    printf("[ibin/init] initalizing...\n");

    HANDLE cntrl = fmkfifo("/var/cntrl/init");

    printf("[init] starting kbc driver\n");
    dexec("/ibin/drivers/kbc", 0);
    waitResp(cntrl);

    printf("[ibin/init] Switching into TTY to VGA mode.\nIf you see this something probably went wrong.\n");

    dexec("/ibin/ttytovga", 0);
    waitResp(cntrl);

    setstdout("/dev/tty0");
    setstdin ("/dev/keyboard");
    setstderr("/dev/tty0");

    printf("[init] now working on tty0\n");

    printf("[init] executing virtual file drivers\n");
    dexec("/ibin/urnd_prov", 0);
    waitResp(cntrl);

    printf("[init] switching to shell\n");

    dexec("/ibin/stsh", 0);

    //register_irq_handler(0x21, hdlrx21);

    while(1);

    return 0;
}
