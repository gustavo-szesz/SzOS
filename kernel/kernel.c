#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>


void kernel_main(void) {
	terminal_initialize();
	printf("SzOS");

  interrupts_initialize();
  keyboard_initialize();

  printf("interrupts on. Type something:\n");

  for (;;) 
  {
    __asm__ volatile ("hlt");
  }
}
