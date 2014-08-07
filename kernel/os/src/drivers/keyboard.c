#include "drivers/keyboard.h"
#include "stdint.h"
#include "idt.h"
#include "ports.h"
#include "vfs.h"
#include "ramfs/fifo.h"
#include "console.h"

uint32_t init_done = 0;
static uint8_t sc_to_kc[][128] = {
    // Normale Scancodes
    {
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
         10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
         20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
         30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
         50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
         60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  84,  00,  00,  86,  87,  88,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00
    },

    // Extended0-Scancodes (werden mit e0 eingeleitet)
    {
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  96,  97,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  99,  00,  00, 100,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00, 102, 103, 104,  00, 105,  00, 106,  00, 107,
        108, 109, 110, 111,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00
    },
};

static char kc_to_char[][128] = {
        {   /* 0 Table: No special key */
            /* 0x00 */  0 ,  0 , '1', '2', '3', '4', '5', '6',
            /* 0x08 */ '7', '8', '9', '0', '-', '`',  8 ,  9 ,
            /* 0x10 */ 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i',
            /* 0x18 */ 'o', 'p', 252, '+','\n',  0 , 'a', 's', //252 = ü
            /* 0x20 */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', 246, //246 = ö
            /* 0x28 */ 228, '#',  0 , '<', 'y', 'x', 'c', 'v', //228 = ä
            /* 0x30 */ 'b', 'n', 'm', ',', '.', '-',  0 , '*',
            /* 0x38 */  0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x40 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7',
            /* 0x48 */ '8', '9', '-', '4', '5', '6', '+', '1',
            /* 0x50 */ '2', '3', '0', '.',  0 ,  0 ,  0 ,  0 ,
            /* 0x58 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x60 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x68 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x70 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x78 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
        },
        {   /* 1 Table: With shift */
            /* 0x00 */  0 , '^', '!', '"', 000, '$', '%', '&', //000 = § TODO find out ISO-8859-1 code
            /* 0x08 */ '/', '(', ')', '=', '?', '`',  8 ,  9 ,
            /* 0x10 */ 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I',
            /* 0x18 */ 'O', 'P', 252, '*','\n',  0 , 'A', 'S', //252 = ü TODO change to upper case
            /* 0x20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', 246, //246 = ö
            /* 0x28 */ 228,'\'',  0 , '>', 'Y', 'X', 'C', 'V', //228 = ä
            /* 0x30 */ 'B', 'N', 'M', ';', ':', '_',  0 , '*',
            /* 0x38 */  0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x40 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7',
            /* 0x48 */ '8', '9', '-', '4', '5', '6', '+', '1',
            /* 0x50 */ '2', '3', '0', '.',  0 ,  0 ,  0 ,  0 ,
            /* 0x58 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x60 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x68 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x70 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
            /* 0x78 */  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
        }
};

#define KF_PRESSED (1 << 0)

uint32_t key_flags[256];

struct res_handle* charout = 0;

static uint8_t translate_scancode(int set, uint16_t scancode)
{
    uint8_t keycode = 0;

    switch (set) {
        // Normal scancodes
        case 0:
            keycode = sc_to_kc[0][scancode];
            break;

        // e0-Scancodes
        case 1:
            keycode = sc_to_kc[1][scancode];
            break;

        // e1-Scancodes
        case 2:
            switch (scancode) {
                // Pause
                case 0x451D:
                    keycode = 119;
                    break;

                default:
                    keycode = 0x0;
            };
            break;
    }

    if (keycode == 0) {
        kprintf("kbc: Unknown Scancode: 0x%x (%d)\n", scancode, set);
    }

    return keycode;
}

static char getchar(uint8_t keycode) {
    uint32_t index = 0;

    if(key_flags[0x2A] & KF_PRESSED) index = 1; //LSHIFT
    if(key_flags[0x36] & KF_PRESSED) index = 1; //RSHIFT

    return kc_to_char[index][keycode];
}

static void send_command(uint8_t command)
{
    do {
        while (inb(0x64) & 0x2) {
        }

        outb(0x60, command);

        while ((inb(0x64) & 0x1) == 0) {
        }
    } while (inb(0x60) == 0xfe);
}

void irq_handler() {
    uint8_t scancode;
    uint8_t keycode = 0;
    int break_code = 0;

    static int      e0_code = 0;
    static int      e1_code = 0;
    static uint16_t e1_prev = 0;

    if(!init_done) return;

    scancode = inb(0x60);

    // Um einen Breakcode handelt es sich, wenn das oberste Bit gesetzt ist und
    // es kein e0 oder e1 fuer einen Extended-scancode ist
    if ((scancode & 0x80) &&
        (e1_code || (scancode != 0xE1)) &&
        (e0_code || (scancode != 0xE0)))
    {
        break_code = 1;
        scancode &= ~0x80;
    }

    if (e0_code) {
        // Fake shift abfangen und ignorieren
        if ((scancode == 0x2A) || (scancode == 0x36)) {
            e0_code = 0;
            return;
        }

        keycode = translate_scancode(1, scancode);
        e0_code = 0;
    } else if (e1_code == 2) {
        // Fertiger e1-Scancode
        // Zweiten Scancode in hoeherwertiges Byte packen
        e1_prev |= ((uint16_t) scancode << 8);
        keycode = translate_scancode(2, e1_prev);
        e1_code = 0;
    } else if (e1_code == 1) {
        // Erstes Byte fuer e1-Scancode
        e1_prev = scancode;
        e1_code++;
    } else if (scancode == 0xE0) {
        // Anfang eines e0-Codes
        e0_code = 1;
    } else if (scancode == 0xE1) {
        // Anfang eines e1-Codes
        e1_code = 1;
    } else {
        // Normaler Scancode
        keycode = translate_scancode(0, scancode);
    }

    if(break_code) {
        key_flags[keycode] &= ~KF_PRESSED;
    }
    else
    {
        key_flags[keycode] |= KF_PRESSED;
    }

    char chr = getchar(keycode);

    if(!break_code && chr) {
        if(vfs_write(charout, &chr, sizeof(char), 1)) {
            kprintf("[keyboard] BUFFER OVERFLOW! \n");
        }
    }
}

void driver_keyboard_init(void)
{
    register_intr_handler(0x21, &irq_handler);

    while (inb(0x64) & 0x1) {
        inb(0x60);
    }

    memset(key_flags, 0, 256 * sizeof(uint32_t));

    // Leds alle ausloeschen
    send_command(0xED);
    send_command(0);

    // Schnellste Wiederholrate
    send_command(0xF3);
    send_command(0);

    send_command(0xF4);

    vfs_create_kfile("/dev/keyboard", ramfs_fifo_driver_struct(), &(uint32_t){512});
    charout = vfs_open("/dev/keyboard", FM_WRITE);

    init_done = 1;
}
