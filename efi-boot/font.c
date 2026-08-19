#include "font.h"
#include "graphics.h"

/* Cada linha é 1 byte = 8 pixels dessa linha do caractere.
   Bit 1 (da esquerda) = pixel aceso. Só dígitos 0-9 e espaço, por enquanto. */
static const uint8_t font_digits[10][8] = {
    {0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0xC6,0x7C}, /* 0 */
    {0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x7E}, /* 1 */
    {0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xFE}, /* 2 */
    {0x7C,0xC6,0x06,0x3C,0x06,0x06,0xC6,0x7C}, /* 3 */
    {0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C}, /* 4 */
    {0xFE,0xC0,0xC0,0xFC,0x06,0x06,0xC6,0x7C}, /* 5 */
    {0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0xC6,0x7C}, /* 6 */
    {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x30}, /* 7 */
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0x7C}, /* 8 */
    {0x7C,0xC6,0xC6,0x7E,0x06,0x06,0x0C,0x78}, /* 9 */
};

void draw_char(char c, int x, int y, uint32_t color) {
    if (c < '0' || c > '9') {
        return; /* por enquanto só sabemos desenhar dígitos; qualquer outra
                    coisa (incluindo espaço) simplesmente não desenha nada */
    }

    const uint8_t *rows = font_digits[c - '0']; /* '5' - '0' = 5, vira índice */

    for (int row = 0; row < 8; row++) {
        uint8_t bits = rows[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                draw_pixel(g_framebuffer, x + col, y + row, g_pixels_per_line, color);
            }
        }
    }
}

void draw_string(const char *text, int x, int y, uint32_t color) {
    int cursor_x = x;
    for (int i = 0; text[i] != '\0'; i++) {
        draw_char(text[i], cursor_x, y, color);
        cursor_x += 9; /* 8 pixels do caractere + 1 de espaçamento */
    }
}

void draw_number(uint32_t value, int x, int y, uint32_t color) {
    char digits[10];
    int count = 0;

    if (value == 0) {
        digits[count++] = '0';
    } else {
        while (value > 0) {
            digits[count++] = '0' + (value % 10); /* pega o último dígito */
            value = value / 10;                    /* descarta ele */
        }
    }

    /* os dígitos foram extraídos de trás pra frente (unidade primeiro),
       então desenhamos o array na ordem INVERSA pra ficar legível */
    int cursor_x = x;
    for (int i = count - 1; i >= 0; i--) {
        draw_char(digits[i], cursor_x, y, color);
        cursor_x += 9;
    }
}
