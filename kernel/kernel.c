#include <stdio.h>
#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>

/* símbolo definido em isr_stubs.S */
extern void irq1(void);

/* print_hex local (o de interrupts.c é static) */
static void print_hex32(uint32_t v) {
    terminal_writestring("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t nibble = (v >> shift) & 0xF;
        terminal_putchar(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
    }
}

void kernel_main(void) {
    terminal_initialize();
    printf("SzOS\n");

    interrupts_initialize();   /* idt + isr + irq */

    printf("irq1=");
    print_hex32((uint32_t)irq1);
    printf("\n");

    interrupts_enable();       /* sti */
    printf("Antes do int 33\n");

    __asm__ volatile ("int $33");

    printf("depois do int 33 - stub OK\n");

    for (;;)
        __asm__ volatile ("hlt");
}