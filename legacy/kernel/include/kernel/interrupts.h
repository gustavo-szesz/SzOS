#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H

/* Sets up the IDT, CPU exception handlers and IRQ/PIC handling, then
 * enables interrupts (sti). Implemented per architecture. */
void interrupts_initialize(void);

void interrupts_enable(void);
void interrupts_disable(void);

/* TEMP: prints EFLAGS + PIC masks directly to the screen. No debugger
 * needed. Call after keyboard_initialize() to check final settled state. */
void interrupts_debug_dump(void);

#endif
