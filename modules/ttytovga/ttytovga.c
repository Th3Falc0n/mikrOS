#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vmmcall.h"
#include "process.h"

#define CMD_SET    0
#define CMD_CLEAR  1
#define CMD_SCROLL 2

struct vga_command {
    uint16_t command;
    uint16_t offset;
    uint16_t value;
};

static int  x     = 0;
static int  y     = 0;
static char color = 0x09;

struct vga_command vgac = { .command = 0, .offset = 0, .value = 0 };

HANDLE vgaController;

void sendCommand(uint16_t c, uint16_t o, uint16_t v) {
    vgac.command = c;
    vgac.offset = o;
    vgac.value = v;

    fwrite(vgaController, &vgac, sizeof(struct vga_command));
}

int main(int argc, char* args[])
{
    vgaController   = fopen("/dev/vga", FM_WRITE);
    HANDLE fifoInpt = fmkfifo("/dev/tty0");

    printf("vgaController: %x\n", vgaController);
    printf("fifoInpt: %x\n", fifoInpt);

    struct vga_command* vgac = malloc(sizeof(struct vga_command));

    setstdout("/dev/tty0");
    setstderr("/dev/tty0");

    printf("[ttytovga] Switched to TTY to VGA (userspace terminal)\n");

    sendCommand(CMD_CLEAR, 0, 0);

    HANDLE initCtrl = fopen("/var/cntrl/init", FM_WRITE);
    fwrite(initCtrl, &(char){'K'}, sizeof(char));
    fclose(initCtrl);

    char nchar = '\0';

    while(1) {
        fread(fifoInpt, &nchar, sizeof(char));

        if ((nchar == '\n') || (x > 79)) {
            x = 0;
            y++;
        }

        if (y > 24) {
            sendCommand(CMD_SCROLL, 0, 0);
            y--;
        }

        if (nchar == '\n') {
            color = 0x09;
            continue;
        }

        if (nchar == 0x11) {
            fread(fifoInpt, &nchar, sizeof(char));
            color = nchar;
            continue;
        }

        uint16_t send = 0;
        char* sndChr = (char*) &send;

        sndChr[0] = nchar;
        sndChr[1] = color;

        color = 0x09;

        sendCommand(CMD_SET, x + y * 80, send);

        x++;
    }

    return 0;
}
