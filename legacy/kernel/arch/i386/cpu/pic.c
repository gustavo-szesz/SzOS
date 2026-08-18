#include "pic.h"
#include "io.h"

#define PIC1            0x20
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2            0xA0
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)

#define PIC_EOI         0x20

#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10

#define ICW4_8086       0x01

void pic_remap(int offset1, int offset2) {
	uint8_t a1 = inb(PIC1_DATA);
	uint8_t a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();

	outb(PIC1_DATA, offset1); io_wait();
	outb(PIC2_DATA, offset2); io_wait();

	outb(PIC1_DATA, 4); io_wait(); /* tell PIC1 there is a slave at IRQ2 (0000 0100) */
	outb(PIC2_DATA, 2); io_wait(); /* tell PIC2 its cascade identity (0000 0010)      */

	outb(PIC1_DATA, ICW4_8086); io_wait();
	outb(PIC2_DATA, ICW4_8086); io_wait();

	outb(PIC1_DATA, a1); /* restore saved masks */
	outb(PIC2_DATA, a2);
}

void pic_send_eoi(uint8_t irq) {
	if (irq >= 8)
		outb(PIC2_COMMAND, PIC_EOI);
	outb(PIC1_COMMAND, PIC_EOI);
}

void pic_set_mask(uint8_t irq_line) {
	uint16_t port = irq_line < 8 ? PIC1_DATA : PIC2_DATA;
	uint8_t line = irq_line < 8 ? irq_line : irq_line - 8;
	uint8_t value = inb(port) | (1 << line);
	outb(port, value);
}

void pic_clear_mask(uint8_t irq_line) {
	uint16_t port = irq_line < 8 ? PIC1_DATA : PIC2_DATA;
	uint8_t line = irq_line < 8 ? irq_line : irq_line - 8;
	uint8_t value = inb(port) & ~(1 << line);
	outb(port, value);
}
