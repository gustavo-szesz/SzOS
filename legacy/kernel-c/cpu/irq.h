#ifndef IRQ_H
#define IRQ_H

#include "./types.h"

/* IRQ Handler - called from assembly */
void irq_handler(u32 irq_number);

#endif
