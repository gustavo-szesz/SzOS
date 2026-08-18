#include "idt.h"
#include <string.h>

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

extern void idt_flush(uint32_t idt_ptr_addr);
  

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low   = base & 0xFFFF;
    idt[num].base_high  = (base >> 16) & 0xFFFF;
    idt[num].sel        = sel;
    idt[num].always0    = 0;
    idt[num].flags      = flags;
}

void idt_install(void) {
  idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
  idtp.base  = (uint32_t) &idt;

  memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

  idt_flush((uint32_t) &idtp);
}
  
