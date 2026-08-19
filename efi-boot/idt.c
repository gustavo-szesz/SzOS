#include "idt.h"

idt_entry_t idt[256] __attribute__((aligned(16)));

void configure_entry_empty(int number, idt_entry_t *table) {
    table[number].addr_low  = 0;
    table[number].sel       = 0;
    table[number].ist       = 0;
    table[number].flags     = 0;
    table[number].addr_mid  = 0;
    table[number].addr_high = 0;
    table[number].reserved  = 0;
}

uint16_t read_current_cs(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return cs;
}

void configure_entry_real(int number, idt_entry_t *table, uint64_t address) {
    table[number].addr_low  = address & 0xFFFF;
    table[number].addr_mid  = (address >> 16) & 0xFFFF;
    table[number].addr_high = (address >> 32) & 0xFFFFFFFF;
    table[number].sel       = read_current_cs();
    table[number].ist       = 0;
    table[number].flags     = 0x8E;
    table[number].reserved  = 0;
}

void initiate_idt(void) {
    for (int i = 0; i < 256; i++) {
        configure_entry_empty(i, idt);
    }

    idt_pointer_t ptr;
    ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    ptr.base  = (uint64_t) &idt;

    asm volatile("lidt %0" : : "m"(ptr));
}
