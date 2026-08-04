#include "vga.h"

static unsigned char *vga_buffer = (unsigned char *)VGA_MEMORY;

void vga_put_pixel(int x, int y, unsigned char color) {
  // Revisar a logica e aprimorar ela 
  if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) {
    return;
  }

  // pixel (x,y) fica no offset (y * largura + x )
  vga_buffer[y * VGA_WIDTH + x] = color;

}

void vga_clear_screen(unsigned char color) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = color;
    }
}
