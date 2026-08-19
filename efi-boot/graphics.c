#include "graphics.h"
#include "sz.c" 

uint32_t *g_framebuffer;
uint32_t  g_pixels_per_line;

void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color) {
    (void) framebuffer; 
    *(g_framebuffer + y * pixels_per_line + x) = color;
}

void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color) {
    (void) framebuffer;
    for (int by = 0; by < size; by++) {
        for (int bx = 0; bx < size; bx++) {
            draw_pixel(g_framebuffer, x + bx, y + by, pixels_per_line, color);
        }
    }
}

void draw_gimp_image(uint32_t *framebuffer, uint32_t screen_x, uint32_t screen_y, uint32_t pixels_per_line) {
    (void) framebuffer;
    uint32_t img_w = gimp_image.width;
    uint32_t img_h = gimp_image.height;
    uint32_t bpp   = gimp_image.bytes_per_pixel;

    for (uint32_t y = 0; y < img_h; y++) {
        for (uint32_t x = 0; x < img_w; x++) {
            uint32_t index = (y * img_w + x) * bpp;
            uint8_t r = gimp_image.pixel_data[index + 0];
            uint8_t g = gimp_image.pixel_data[index + 1];
            uint8_t b = gimp_image.pixel_data[index + 2];
            uint32_t color = (r << 16) | (g << 8) | b;

            uint32_t fb_x = screen_x + x;
            uint32_t fb_y = screen_y + y;
            g_framebuffer[fb_y * pixels_per_line + fb_x] = color;
        }
    }
}

void draw_logo(void) {
    const char *logo[] = {
        " >>========================================<< ",
        " ||  .-')      .-') _               .-')   || ",
        " || ( OO ).   (  OO) )             ( OO ). || ",
        " ||(_)---\\_),(_)----. .-'),-----. (_)---\\_)|| ",
        " ||/    _ | |       |( OO'  .-.  '/    _ | || ",
        " ||\\  :` `. '--.   / /   |  | |  |\\  :` `. || ",
        " || '..`''.)(_/   /  \\_) |  |\\|  | '..`''.)|| ",
        " ||.-._)   \\ /   /___  \\ |  | |  |.-._)   \\|| ",
        " ||\\       /|        |  `'  '-'  '\\       /|| ",
        " || `-----' `--------'    `-----'  `-----' || ",
        " >>========================================<< ",
        NULL
    };
    for (int line = 0; logo[line] != NULL; line++) {
        printf("%s\n", logo[line]);
    }
}
