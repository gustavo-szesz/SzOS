#ifndef IDT_H
#define IDT_H

#include <uefi.h>

typedef struct {
    uint16_t addr_low;
    uint16_t sel;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t addr_mid;
    uint32_t addr_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_pointer_t;

/* a tabela em si -- "extern" avisa outros arquivos que ela existe,
   sem recriar ela de novo (só é criada de verdade em idt.c) */
extern idt_entry_t idt[256];

void configure_entry_empty(int number, idt_entry_t *table);
void configure_entry_real(int number, idt_entry_t *table, uint64_t address);
void initiate_idt(void);
uint16_t read_current_cs(void);

#endif
