#ifndef MOUSE_H
#define MOUSE_H

#include <uefi.h>

extern volatile int g_mouse_x;
extern volatile int g_mouse_y;

void mouse_init_state(int start_x, int start_y);
__attribute__((interrupt)) void handler_mouse(void *frame);
void wait_buffer_entry_free();
void wait_buffer_out_fill();
void write_command(uint8_t byte);
void write_data(uint8_t byte);
int read_data(void);
void activate_mouse(void);
void copy_memory(uint32_t *destiny, uint32_t *origem, uint32_t many_bytes);


#endif