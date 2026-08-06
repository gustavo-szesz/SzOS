#include "idt.h"

idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void set_idt_gate(int n, u32 handler) {
    idt[n].low_offset  = (u16)(handler & 0xFFFF);
    idt[n].high_offset = (u16)((handler >> 16) & 0xFFFF);
    idt[n].sel         = KERNEL_CS;
    idt[n].always0     = 0;
    idt[n].flags       = 0x8E; /* present, ring 0, 32-bit interrupt gate */
}

void set_idt() {
    idt_reg.limit = (sizeof(idt_gate_t) * IDT_ENTRIES) - 1;
    idt_reg.base  = (u32)&idt;
    asm volatile ("lidt %0" : : "m" (idt_reg));
}