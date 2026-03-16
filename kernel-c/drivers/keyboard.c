#include "./keyboard.h"
#include "./ports.h"
#include "../cpu/idt.h"
#include "../drivers/screen.h"

static unsigned char key_buffer[256];
static unsigned char last_scancode = 0;


void keyboard_init() {
  int i;
  for (i = 0; i < 256; i++) {
    key_buffer[i] = 0;
  }

  set_idt_gate(33, (u32)keyboard_isr);

  kprint("Keyboard initialized\n");
}

//key pressed
int key_pressed(unsigned char key_code) {
  return key_buffer[key_code] ? 1 : 0;
}

unsigned char get_last_scancode() {
  return last_scancode;
}

void keyboard_handler(unsigned char scancode) {
  last_scancode = scancode;

  if (scancode & 0x80) {
    key_buffer[scancode & 0x7F] = 0;
  } else {
    key_buffer[scancode] = 1;
  }
}

extern void keyboard_isr();

/*
void print_letter(u8 scancode) {
  switch (scancode) {
    case 0x11:
      kprint("W");
      break;
    case 0x1E:
      kprint("A");
      break;
    case 0x1F:
      kprint("S");
      break;
    case 0x20:
      kprint("D");
      break;
    case 0x48:
      kprint("UP");
      break;
    case 0x50:
      kprint("DOWN");
      break;
    case 0x4B:
      kprint("LEFT");
      break;
    case 0x4D:
      kprint("RIGHT");
      break;

    default:
      if (scancode <= 0x7f) {
        kprint("Unknown key down");
      } else if (scancode <= 0x39 + 0x80) {
        kprint("key up ");
        print_letter(scancode - 0x80);
      } else kprint("Unknown key up");
      break;
  }
}
*/

