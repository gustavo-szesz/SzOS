#include "isr.h"
#include "idt.h"

#include <kernel/tty.h>

/* isr0..isr31: defined in isr_stubs.S, one tiny stub per exception. */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

/* Shared table: slots 0-31 are CPU exceptions, 32-47 are IRQs (irq.c fills those). */
static isr_t interrupt_handlers[256];

static const char *exception_messages[32] = {
	"Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
	"Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
	"Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
	"Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
	"Coprocessor Fault", "Alignment Check", "Machine Check", "SIMD Floating-Point",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved", "Reserved", "Reserved",
};

/* SzOS's printf() currently only supports %c/%s, so we roll a tiny
 * decimal writer here instead of depending on %u. */
static void write_uint(uint32_t n) {
	char digits[10];
	int i = 0;
	if (n == 0) {
		terminal_putchar('0');
		return;
	}
	while (n > 0) {
		digits[i++] = '0' + (n % 10);
		n /= 10;
	}
	while (i > 0)
		terminal_putchar(digits[--i]);
}

void isr_register_handler(uint8_t n, isr_t handler) {
	interrupt_handlers[n] = handler;
}

void isr_install(void) {
	idt_set_gate(0,  (uint32_t) isr0,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(1,  (uint32_t) isr1,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(2,  (uint32_t) isr2,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(3,  (uint32_t) isr3,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(4,  (uint32_t) isr4,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(5,  (uint32_t) isr5,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(6,  (uint32_t) isr6,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(7,  (uint32_t) isr7,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(8,  (uint32_t) isr8,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(9,  (uint32_t) isr9,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(10, (uint32_t) isr10, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(11, (uint32_t) isr11, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(12, (uint32_t) isr12, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(13, (uint32_t) isr13, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(14, (uint32_t) isr14, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(15, (uint32_t) isr15, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(16, (uint32_t) isr16, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(17, (uint32_t) isr17, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(18, (uint32_t) isr18, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(19, (uint32_t) isr19, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(20, (uint32_t) isr20, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(21, (uint32_t) isr21, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(22, (uint32_t) isr22, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(23, (uint32_t) isr23, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(24, (uint32_t) isr24, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(25, (uint32_t) isr25, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(26, (uint32_t) isr26, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(27, (uint32_t) isr27, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(28, (uint32_t) isr28, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(29, (uint32_t) isr29, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(30, (uint32_t) isr30, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(31, (uint32_t) isr31, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
}

void isr_handler(struct registers *regs) {
	if (interrupt_handlers[regs->int_no] != 0) {
		interrupt_handlers[regs->int_no](regs);
		return;
	}

	if (regs->int_no < 32) {
		terminal_writestring("\nPANIC: ");
		terminal_writestring(exception_messages[regs->int_no]);
		terminal_writestring(" (interrupt ");
		write_uint(regs->int_no);
		terminal_writestring(", error code ");
		write_uint(regs->err_code);
		terminal_writestring(")\n");
		__asm__ volatile ("cli");
		for (;;)
			__asm__ volatile ("hlt");
	}
}
