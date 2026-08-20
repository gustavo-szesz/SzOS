#ifndef HARDWARE_H
#define HARDWARE_H

// ================= PIC ====================
#define PIC1_COMMAND        0x20
#define PIC1_DATA           0x21
#define PIC2_COMMAND        0xA0
#define PIC2_DATA           0xA1

#define PIC_EOI             0x20
#define PIC_INIT            0x11
#define PIC_ICW4_8086       0x01

#define IRQ_BASE_MASTER     32
#define IRQ_BASE_SLAVE      40

#define PIC1_MASK_DEFAULT   0xF8
#define PIC2_MASK_DEFAULT   0xEF

// =============== PIT ===================
#define PIT_COMMAND         0x43
#define PIT_CHANNELO        0x40
#define PIT_FREQUENCY_HZ    1193182
#define PIT_MODE_RATE_GEN   0x36

// =============== PS2/2 =================
#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64

#define PS2_STATUS_OUTPUT   0x01
#define PS2_STATUS_INPUT    0x01

#define PS2_CMD_ENABLE_AUX  0xA8
#define PS2_CMD_READ_CFG    0x20
#define PS2_CMD_WRITE_CFG   0x60
#define PS2_CMD_WRITE_MOUSE 0xD4

#define MOUSE_CMD_DEFAULTS  0xF6
#define MOUSE_CMD_ENABLE    0xF4

// =============== KEYBOARD ============
#define KEY_UP              0x48
#define KEY_DOWN            0x50
#define KEY_LEFT            0x4B
#define KEY_RIGHT           0x4D
#define KEY_SPACE           0x39

// ================ IDT ================
#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RINGO      0x00
#define IDT_FLAG_INTERRUPT  0x0E
#define IDT_TYPE_INTERRUPT  (IDT_FLAG_PRESENT | IDT_FLAG_RINGO | IDT_FLAG_INTERRUPT)

#define PAGE_SIZE           4096

#endif