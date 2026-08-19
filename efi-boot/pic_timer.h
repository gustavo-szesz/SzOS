#ifndef PIC_TIMER_H
#define PIC_TIMER_H

#include <uefi.h>

extern volatile uint64_t g_ticks;
extern volatile int g_last_key;

void remap_pic(void);
void program_pit(uint32_t freq_desired);
void delay_real(uint64_t ticks_for_wait);

__attribute__((interrupt)) void handler_time(void *frame);
__attribute__((interrupt)) void handler_keyboard(void *frame);
__attribute__((interrupt)) void handler_interruption_test(void *frame);

#endif
