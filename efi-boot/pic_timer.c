#include "include/pic_timer.h"
#include "include/ports.h"
#include "include/graphics.h"
#include "include/hardware.h"

volatile uint64_t g_ticks = 0;
volatile int g_last_key = 0;

void remap_pic(void) {
    outb(PIC1_COMMAND, PIC_INIT);
    outb(PIC2_COMMAND, PIC_INIT);

    outb(PIC1_DATA, IRQ_BASE_MASTER);
    outb(PIC2_DATA, IRQ_BASE_SLAVE);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, PIC_ICW4_8086);
    outb(PIC2_DATA, PIC_ICW4_8086);

    outb(PIC1_DATA, PIC1_MASK_DEFAULT); 
    outb(PIC1_DATA, PIC2_MASK_DEFAULT);
}

void send_eoi(uint8_t irq)
{
    if (irq >= 8){
        outb(PIC2_COMMAND, PIC_EOI);
    } outb(PIC1_COMMAND, PIC_EOI);
}

void program_pit(uint32_t freq_desired) {
    uint32_t div = PIT_FREQUENCY_HZ / freq_desired;

    outb(PIT_COMMAND, PIT_MODE_RATE_GEN);
    outb(PIT_CHANNELO, div & 0xFF);
    outb(PIT_CHANNELO, (div >> 8) & 0xFF);
}

void delay_real(uint64_t ticks_for_wait) {
    uint64_t ticks_initial = g_ticks;
    while ((g_ticks - ticks_initial) < ticks_for_wait) {
        continue;
    }
}

__attribute__((interrupt))
void handler_time(void *frame) {
    g_ticks++;
    send_eoi(0);
    (void) frame;
}

__attribute__((interrupt))
void handler_keyboard(void *frame) {
    g_last_key = inb(PS2_DATA_PORT);
    send_eoi(1);
    (void) frame;
}

__attribute__((interrupt))
void handler_interruption_test(void *frame) {
    draw_pixel(g_framebuffer, 930, 0, g_pixels_per_line, 0x00FFFF00);
    (void) frame;
}
