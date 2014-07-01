#include "keyboard.h"
#include "idt.h"
#include "ports.h"
#include "console.h"
#include "pmm.h"

static uint8_t sc_to_kc[][128] = {
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
    {
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  97,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00, 100,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00, 102, 103, 104,  00, 105,  00, 106,  00, 107,
        108, 109, 110, 111,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00
    },
};

static int init_done = 0;

static uint8_t buffer[4096];
static int bfindex = 0;

uint8_t translate_scancode(int set, uint16_t scancode)
{
  uint8_t keycode = 0;

  switch (set) {
    case 0:
      keycode = sc_to_kc[0][scancode];
      break;
    case 1:
      keycode = sc_to_kc[1][scancode];
      break;
    case 2:
      switch (scancode) {
        case 0x451D:
          keycode = 119;
          break;

        default:
          keycode = 0x0;	
      };
      break;
  }

  if (keycode == 0) {
      kprintf("kbc: Unbekannter Scancode: 0x%x (%d)\n", scancode, set);
  }

  return keycode;
}

void keyboard_handler() {    
  return;
  uint8_t scancode;
  uint8_t keycode = 0;
  int break_code = 0;

  static int      e0_code = 0;
  static int      e1_code = 0;
  static uint16_t  e1_prev = 0;

  scancode = inb(0x60);

  if (!init_done) {
    return;
  }

  if ((scancode & 0x80) &&
      (e1_code || (scancode != 0xE1)) &&
      (e0_code || (scancode != 0xE0)))
  {
      break_code = 1;
      scancode &= ~0x80;
  }

  if (e0_code) {
      if ((scancode == 0x2A) || (scancode == 0x36)) {
          e0_code = 0;
          return;
      }
      keycode = translate_scancode(1, scancode);
      e0_code = 0;
  } else if (e1_code == 2) {
      e1_prev |= ((uint16_t) scancode << 8);
      keycode = translate_scancode(2, e1_prev);
      e1_code = 0;
  } else if (e1_code == 1) {
      e1_prev = scancode;
      e1_code++;
  } else if (scancode == 0xE0) {
      e0_code = 1;
  } else if (scancode == 0xE1) {
      e1_code = 1;
  } else {
      keycode = translate_scancode(0, scancode);
  }

	if(keycode != 0x0) {
		//buffer[bfindex] = keycode;
		//bfindex++;	
	}
}

char getc() {
	char ret = buffer[0];
	
	int i;
	for(i = 0; i < bfindex; i++) {
		buffer[i] = buffer[i+1];	
	}
	bfindex--;

	return ret;
}

void send_command(uint8_t command) {
	while((inb(0x64) & 0x2)) { }
	outb(0x60, command);
}

void init_keyboard_drv() {
	register_intr_handler(0x21, &keyboard_handler);

	while(inb(0x64) & 0x1) {
		inb(0x60);
	}

	send_command(0xF4);

	init_done = 1;
}
