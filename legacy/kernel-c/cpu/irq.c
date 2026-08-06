#include "./irq.h"
#include "../drivers/ports.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"


void irq_handler(u32 irq_number) {
  if (irq_number == 33) {
    u8 scancode = port_byte_in(0x60);
    keyboard_handler(scancode);
  }

  if (irq_number >= 40){
    port_byte_out(0xA0, 0x20);
  }
  port_byte_out(0x20, 0x20);
}
