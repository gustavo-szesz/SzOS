#ifndef PIC_H
#define PIC_H

#include "../cpu/types.h"

/* PIC (Programmable Interrupt Controller) initialization */
void pic_init();

/* Enable specific IRQ */
void pic_enable_irq(u8 irq);

/* Disable specific IRQ */
void pic_disable_irq(u8 irq);

#endif
