#include "pic.h"
#include "ports.h"
#include "../drivers/screen.h"

/* PIC I/O Ports */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

/* ICW1 - Initialization Control Word 1 */
#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10

/* ICW4 - Initialization Control Word 4 */
#define ICW4_8086       0x01
#define ICW4_AUTO       0x02
#define ICW4_BUF_MASTER 0x04
#define ICW4_BUF_SLAVE  0x08
#define ICW4_SFNM       0x10

#define PIC_EOI         0x20

/* Initialize PIC */
void pic_init() {
    u8 mask1, mask2;
    
    /* Save current masks */
    mask1 = port_byte_in(PIC1_DATA);
    mask2 = port_byte_in(PIC2_DATA);
    
    /* ICW1 - Start initialization */
    port_byte_out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    port_byte_out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    /* ICW2 - Set interrupt vectors */
    port_byte_out(PIC1_DATA, 0x20);  /* Master PIC vectors: 32-39 */
    port_byte_out(PIC2_DATA, 0x28);  /* Slave PIC vectors: 40-47 */
    
    /* ICW3 - Setup cascading */
    port_byte_out(PIC1_DATA, 0x04);  /* Master has slave on IRQ2 */
    port_byte_out(PIC2_DATA, 0x02);  /* Slave is on Master's IRQ2 */
    
    /* ICW4 - Setup 8086 mode */
    port_byte_out(PIC1_DATA, ICW4_8086);
    port_byte_out(PIC2_DATA, ICW4_8086);
    
    /* Restore masks (enable all interrupts) */
    port_byte_out(PIC1_DATA, 0x00);  /* Enable all Master interrupts */
    port_byte_out(PIC2_DATA, 0x00);  /* Enable all Slave interrupts */
    
    kprint("PIC initialized\n");
}

/* Enable specific IRQ */
void pic_enable_irq(u8 irq) {
    u16 port;
    u8 value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = port_byte_in(port);
    value &= ~(1 << irq);
    port_byte_out(port, value);
}

/* Disable specific IRQ */
void pic_disable_irq(u8 irq) {
    u16 port;
    u8 value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = port_byte_in(port);
    value |= (1 << irq);
    port_byte_out(port, value);
}
