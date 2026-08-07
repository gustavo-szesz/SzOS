#ifndef ARCH_I386_CPU_IRQ_H
#define ARCH_I386_CPU_IRQ_H

#include <stdint.h>
#include "isr.h"

#define IRQ_BASE 0x20

void irq_install(void);

void irq_register_handler(uint8_t irq, isr_t handler);

void irq_handler(struct registers *regs);

#endif // !ARCH_I386_CPU_IRQ_H

