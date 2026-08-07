#include "irq.h"
#include "idt.h"
#include "pic.h"

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);  extern void irq3(void);
extern void irq4(void);  extern void irq5(void);  extern void irq6(void);  extern void irq7(void);
extern void irq8(void);  extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);

void irq_install(void) {
	/* Move IRQ0-15 from the BIOS defaults (0x08-0x0F / 0x70-0x77, which
	 * overlap CPU exceptions) to 0x20-0x2F. */
	pic_remap(IRQ_BASE, IRQ_BASE + 8);

	idt_set_gate(32, (uint32_t) irq0,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(33, (uint32_t) irq1,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(34, (uint32_t) irq2,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(35, (uint32_t) irq3,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(36, (uint32_t) irq4,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(37, (uint32_t) irq5,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(38, (uint32_t) irq6,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(39, (uint32_t) irq7,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(40, (uint32_t) irq8,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(41, (uint32_t) irq9,  KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(42, (uint32_t) irq10, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(43, (uint32_t) irq11, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(44, (uint32_t) irq12, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(45, (uint32_t) irq13, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(46, (uint32_t) irq14, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
	idt_set_gate(47, (uint32_t) irq15, KERNEL_CS, IDT_FLAG_INTERRUPT_GATE);
}

void irq_register_handler(uint8_t irq, isr_t handler) {
	isr_register_handler(IRQ_BASE + irq, handler);
	pic_clear_mask(irq);
}

void irq_handler(struct registers *regs) {
	/* int_no is 32-47 here; isr_handler() already looked the handler up
	 * in the shared table and, if int_no >= 32, falls through to here
	 * without panicking. We just need to send the EOI. */
	isr_handler(regs);
	pic_send_eoi(regs->int_no - IRQ_BASE);
}
