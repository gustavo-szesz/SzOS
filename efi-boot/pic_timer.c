#include "pic_timer.h"
#include "ports.h"
#include "graphics.h"

volatile uint64_t g_ticks = 0;
volatile int g_last_key = 0;

void remap_pic(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 32);
    outb(0xA1, 40);

    outb(0x21, 4);
    outb(0xA1, 2);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xF8); 
    outb(0xA1, 0xEF);
}

void program_pit(uint32_t freq_desired) {
    uint32_t div = 1193182 / freq_desired;

    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, (div >> 8) & 0xFF);
}

void delay_real(uint64_t ticks_for_wait) {
    uint64_t ticks_initial = g_ticks;
    while ((g_ticks - ticks_initial) < ticks_for_wait) {
        continue;
    }
}

__attribute__((interrupt))
void handler_time(void *frame) {
    g_ticks = g_ticks + 1;
    outb(0x20, 0x20);
    (void) frame;
}

__attribute__((interrupt))
void handler_keyboard(void *frame) {
    g_last_key = inb(0x60);
    outb(0x20, 0x20);
    (void) frame;
}

__attribute__((interrupt))
void handler_interruption_test(void *frame) {
    draw_pixel(g_framebuffer, 930, 0, g_pixels_per_line, 0x00FFFF00);
    (void) frame;
}
