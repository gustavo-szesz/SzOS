#include "include/mouse.h"
#include "include/hardware.h"
#include "include/pic_timer.h"
#include "include/ports.h"

// 0x60 data (read, write bytes of data)
// PS2_STATUS_PORT command (write and read)

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
    static int byte_index = 0;
        static int packet[3];

        packet[byte_index++] = inb(PS2_DATA_PORT);

        if (byte_index == 3) {
            int8_t dx = (int8_t)packet[1];
            int8_t dy = (int8_t)packet[2];

            g_mouse_x += dx;
            g_mouse_y -= dy;     /* eixo Y invertido */

            /* Clipping simples */
            if (g_mouse_x < 0) g_mouse_x = 0;
            if (g_mouse_y < 0) g_mouse_y = 0;
            /* (adicione limites de tela quando tiver width/height) */

            byte_index = 0;
        }

        send_eoi(12);            /* IRQ12 */
        (void)frame;
}

void  wait_input_buffer_clear(){
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT) != 0) {
        //delay_real(60);
        ;
    }
}

void wait_input_buffer_full(void){
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT) == 0) {
        //delay_real(60);
    }
}

void ps2_write_command(uint8_t byte){
    wait_input_buffer_clear();
    outb(PS2_STATUS_PORT, byte);
}

void ps2_write_data(uint8_t byte){
    wait_input_buffer_clear();
    outb(PS2_DATA_PORT, byte);
}

int read_data(){
    wait_input_buffer_full();
    return inb(PS2_DATA_PORT);
}

void activate_mouse(){
    ps2_write_command(PS2_CMD_ENABLE_AUX);

    ps2_write_command(PS2_CMD_READ_CFG);
    int config = read_data();

    config = config | 0x02;  // turn bit 1
    config = config & ~0x20;

    ps2_write_command(PS2_CMD_WRITE_CFG);
    ps2_write_data(config);

    ps2_write_command(PS2_CMD_WRITE_MOUSE);
    ps2_write_data(MOUSE_CMD_DEFAULTS);

    read_data();
    
    ps2_write_command(PS2_CMD_WRITE_MOUSE);
    write_data(MOUSE_CMD_ENABLE);
    read_data();
}