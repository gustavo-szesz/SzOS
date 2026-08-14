#include <kernel/interrupts.h>

#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "io.h"

void interrupts_initialize(void) {
	idt_install();
    isr_install();
    irq_install();

    outb(0x21, 0xFF);   /* mascara master */
    outb(0xA1, 0xFF);   /* mascara slave  */

    outb(0x21, 0xFD);   /* libera só IRQ1 (teclado) = bit 1 */
}

void interrupts_enable(void) {
	__asm__ volatile ("sti");
}

void interrupts_disable(void) {
	__asm__ volatile ("cli");
}

/* --- diagnostic dump, no debugger required --- */
#include <kernel/tty.h>
#include "io.h"


static void print_hex32(uint32_t v) {
	terminal_writestring("0x");
	for (int shift = 28; shift >= 0; shift -= 4) {
		uint8_t nibble = (v >> shift) & 0xF;
		terminal_putchar(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
	}
}

static void print_hex8(uint8_t v) {
	terminal_writestring("0x");
	terminal_putchar("0123456789ABCDEF"[(v >> 4) & 0xF]);
	terminal_putchar("0123456789ABCDEF"[v & 0xF]);
}

void interrupts_debug_dump(void) {
	uint32_t eflags;
	__asm__ volatile ("pushf\n\tpop %0" : "=r"(eflags));

	uint8_t pic1_mask = inb(0x21); /* PIC1 (master) IMR */
	uint8_t pic2_mask = inb(0xA1); /* PIC2 (slave) IMR  */

	terminal_writestring("\n[debug] EFLAGS=");
	print_hex32(eflags);
	terminal_writestring(eflags & 0x200 ? " (IF=1, interrupts ON)" : " (IF=0, interrupts OFF!)");
	terminal_writestring("\n[debug] PIC1 mask=");
	print_hex8(pic1_mask);
	terminal_writestring(pic1_mask & 0x02 ? " (IRQ1/keyboard MASKED!)" : " (IRQ1/keyboard unmasked)");
	terminal_writestring("\n[debug] PIC2 mask=");
	print_hex8(pic2_mask);
	terminal_writestring("\n");
}
