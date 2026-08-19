#include "mouse.h"
#include "pic_timer.h"
#include "ports.h"

// 0x60 data (read, write bytes of data)
// 0x64 command (write and read)

volatile int g_mouse_x = 0;
volatile int g_mouse_y = 0;
volatile int g_mouse_screen_w = 0;
volatile int g_mouse_screen_h = 0;

static int mouse_byte_atual = 0;
static int package[3];

void mouse_init_state(int start_x, int start_y) {
    g_mouse_x = start_x;
    g_mouse_y = start_y;
}

__attribute__((interrupt))
void handler_mouse(void *frame) {
    int data = inb(0x60);

    package[mouse_byte_atual] = data;
    mouse_byte_atual = mouse_byte_atual + 1;

    if (mouse_byte_atual == 3) {
        int delta_x = package[1];
        int delta_y = package[2];

        g_mouse_x = g_mouse_x + delta_x;
        g_mouse_y = g_mouse_y - delta_y; /* Mouse send the signal upside down */

        if (g_mouse_x < 0) { g_mouse_x = 0; }
        if (g_mouse_y < 0) { g_mouse_y = 0; }

        mouse_byte_atual = 0;
    }

    outb(0x20, 0x20); /* EOI: master */
    outb(0xA0, 0x20); /* EOI: slave */
    (void) frame;
}

void wait_buffer_entry_free(){
    while ((inb(0x64) & 0x02) != 0) {
        //delay_real(60);
    }
}

void wait_buffer_out_fill(){
    while ((inb(0x64) & 0x01) == 0) {
        //delay_real(60);
    }
}

void write_command(uint8_t byte){
    wait_buffer_entry_free();
    outb(0x64, byte);
}

void write_data(uint8_t byte){
    wait_buffer_entry_free();
    outb(0x60, byte);
}

int read_data(){
    wait_buffer_out_fill();
    return inb(0x60);
}

void activate_mouse(){
    write_command(0xA8);

    write_command(0x20);
    int config = read_data();

    config = config | 0x02;  // turn bit 1
    config = config & ~0x20;

    write_command(0x60);
    write_data(config);

    write_command(0xD4);
    write_data(0xF6);

    read_data();
    
    write_command(0xD4);
    write_data(0xF4);
    read_data();
}