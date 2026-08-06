#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/types.h"

// drivers/keyboard.h
#define KEY_ESC       0x01
#define KEY_1         0x02
#define KEY_2         0x03
#define KEY_3         0x04
#define KEY_4         0x05
#define KEY_5         0x06
#define KEY_6         0x07
#define KEY_7         0x08
#define KEY_8         0x09
#define KEY_9         0x0A
#define KEY_0         0x0B
#define KEY_Q         0x10
#define KEY_W         0x11
#define KEY_E         0x12
#define KEY_R         0x13
#define KEY_T         0x14
#define KEY_A         0x1E
#define KEY_S         0x1F
#define KEY_D         0x20
#define KEY_F         0x21
#define KEY_G         0x22
#define KEY_H         0x23
#define KEY_Z         0x2C
#define KEY_X         0x2D
#define KEY_C         0x2E
#define KEY_V         0x2F
#define KEY_B         0x30
#define KEY_SPACE     0x39
#define KEY_UP        0x48
#define KEY_DOWN      0x50
#define KEY_LEFT      0x4B
#define KEY_RIGHT     0x4D
#define KEY_ENTER     0x1C
#define KEY_LSHIFT    0x2A
#define KEY_RSHIFT    0x36
#define KEY_LCTRL     0x1D

// Ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void keyboard_init();
int key_pressed(unsigned char key_code);
void keyboard_handler(unsigned char scancode);

unsigned char get_last_scancode();

extern void keyboard_isr();
#endif
