#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <stdbool.h>

void keyboard_initialize(void);

bool keyboard_read(char *out_c);

bool keyboard_ctrl_pressed(void);
bool keyboard_alt_pressed(void);
bool keyboard_shift_pressed(void);

#endif // !_KERNEL_KEYBOARD_H
