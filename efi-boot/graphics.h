#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <uefi.h>

extern uint32_t *g_framebuffer;
extern uint32_t  g_pixels_per_line;

void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color);
void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color);
void draw_logo(void);
void draw_gimp_image(uint32_t *framebuffer, uint32_t screen_x, uint32_t screen_y, uint32_t pixels_per_line);

#endif
