#include "../drivers/screen.h"
#include "util.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../drivers/keyboard.h"
#include "../drivers/pic.h"

/* void print_all_rows(void) {
  for (int i = 0; i < 24; i++) {
    char str[255];
    int_to_ascii(i, str);
    kprint_at(str, 0, i);
  }
  kprint_at("This text forces the kernel to scroll. Row 0 will disappear. ", 60, 24);
  kprint("And with this text, the kernel will scroll again, and row 1 will disappear too!");
}
 */

void test_keyboard() {
  kprint("\n=== Keyboard Test ===\n");
  kprint("Press keys to test (W, A, S, D, SPACE, ESC):\n");
  kprint("Press ESC to exit\n\n");

  int loop = 1;
  while (loop) {
    /* Check each key */
    if (key_pressed(KEY_W)) {
      kprint_at("W pressed", 0, 3);
    } else {
      kprint_at("         ", 0, 3);
    }

    if (key_pressed(KEY_A)) {
      kprint_at("A pressed", 10, 3);
    } else {
      kprint_at("         ", 10, 3);
    }


    if (key_pressed(KEY_S)) {
      kprint_at("S pressed", 20, 3);
    } else {
      kprint_at("         ", 20, 3);
    }

    if (key_pressed(KEY_D)) {
      kprint_at("D pressed", 30, 3);
    } else {
      kprint_at("         ", 30, 3);
    }
    
    if (key_pressed(KEY_SPACE)) {
      kprint_at("SPACE pressed", 40, 3);
    } else {
      kprint_at("         ", 40, 3);
    }

    if (key_pressed(KEY_ESC)) {
      loop = 0;
      kprint("\nTest ended. Exiting...\n");
    }
  }
}

void main() {
  kprint("SZ_OS Kernel Loaded.\n");
  kprint("Welcome to SZ_OS!\n");
  kprint("Initializing...\n\n\n");
  
  kprint_at("Kernel Initialized Successfully.", 50, 10);

  /* Initialize interrupt handling */
  isr_install();
  pic_init();
  keyboard_init();
  
  /* Enable interrupts globally */
  __asm__ __volatile__ ("sti");

  kprint("Testing exceptions...\n");
  __asm__ __volatile__ ("int $2");
  __asm__ __volatile__ ("int $3");

  kprint("\nStarting keyboard test...\n");
  kprint("The loop is running. Press keys:\n");
  test_keyboard();

  kprint("Kernel shutting down. \n");

  while(1);
}
