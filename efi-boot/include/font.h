#ifndef FONT_H
#define FONT_H

#include <uefi.h>

void draw_char(char c, int x, int y, uint32_t color);
void draw_string(const char *text, int x, int y, uint32_t color);
void draw_number(uint32_t value, int x, int y, uint32_t color);

#endif
