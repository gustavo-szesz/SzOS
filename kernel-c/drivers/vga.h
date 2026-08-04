#ifndef VGA_H
#define VGA_H

#include "../cpu/types.h"

#define VGA_WIDTH  320
#define VGA_HEIGHT 200
#define VGA_MEMORY 0xA0000

void vga_put_pixel(int x, int y, unsigned char color);
void vga_clear_screen(unsigned char color);

#endif // !VGA_H

