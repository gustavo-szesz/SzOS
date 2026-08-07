#include <kernel/interrupts.h>

#include "idt.h"
#include "isr.h"
#include "irq.h"

void interrupts_initialize(void) {
  idt_install();
  isr_install();
  isr_install();
  // __asm__ volatile ("sti");
}

void interrupts_enable(void) {
  __asm__ volatile ("sti");
}

void interrupts_disable(void){
  __asm__ volatile ("cli");
}
