#include "../drivers/screen.h"
#include "util.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"

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
void main() {
  kprint("SZ_OS Kernel Loaded.\n");
  kprint("Welcome to SZ_OS!\n");
  kprint("Initializing...\n\n\n");
  
  kprint_at("Kernel Initialized Successfully.", 50, 10);

  isr_install();
  __asm__ __volatile__ ("int $2");
  __asm__ __volatile__ ("int $3");

}
