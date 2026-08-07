#include <stddef.h>

#include <kernel/keyboard.h>
#include <kernel/tty.h>

#include "../cpu/io.h"
#include "../cpu/irq.h"
#include "../cpu/isr.h"

#define PS2_DATA_PORT   0x60
#define PS2_STATUS_PORT 0x64
#define PS2_STATUS_OUTPUT_FULL 0x01
#define PS2_STATUS_INPUT_FULL  0x02

#define KBD_CMD_SET_LEDS 0xED 
#define KBD_RESP_ACK     0xFA 
#define KBD_RESP_RESEND  0xFE 

#define KBD_LED_SCROLL_LOCK 0x01 
#define KBD_LED_NUM_LOCK    0x02 
#define KBD_LED_CAPS_LOCK   0x04 

#define KBD_IRQ       1

#define KBD_BUFFER_SIZE 256

static char buffer[KBD_BUFFER_SIZE];
static volatile size_t buf_head = 0; /* next write position  */
static volatile size_t buf_tail = 0; /* next read position    */

static bool shift_pressed = false;
static bool ctrl_pressed  = false;
static bool alt_pressed   = false;
static bool caps_lock     = false;

/* US QWERTY, scancode set 1, unshifted. Index = scancode (make code). */
static const char scancode_ascii[128] = {
	0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', /* 0x00-0x09 */
	'9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r', /* 0x0A-0x13 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   /* 0x14-0x1D (0x1D = left ctrl) */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 0x1E-0x27 */
	'\'','`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',   /* 0x28-0x31 (0x2A = left shift) */
	'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,    /* 0x32-0x3B (0x36 = right shift) */
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 0x3C-0x45 (F-keys, numlock...) */
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,
};

/* Same table, shifted (letters uppercased, symbol row shifted). */
static const char scancode_ascii_shift[128] = {
	0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
	'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,
};

#define SCANCODE_LSHIFT       0x2A
#define SCANCODE_RSHIFT       0x36
#define SCANCODE_LCTRL        0x1D
#define SCANCODE_LALT         0x38
#define SCANCODE_LSHIFT_REL   0xAA
#define SCANCODE_RSHIFT_REL   0xB6
#define SCANCODE_CAPSLOCK     0x3A
#define SCANCODE_RELEASE_BIT  0x80
#define SCANCODE_EXTENDED_PREFIX 0xE0
#define SCANCODE_EXT_RCTRL    0x1D 
#define SCANCODE_EXT_RALT     0x38

static void buffer_push(char c) {
	size_t next = (buf_head + 1) % KBD_BUFFER_SIZE;
	if (next == buf_tail) /* buffer full, drop the oldest byte */
		buf_tail = (buf_tail + 1) % KBD_BUFFER_SIZE;
	buffer[buf_head] = c;
	buf_head = next;
}

bool keyboard_read(char *out_c) {
	if (buf_tail == buf_head)
		return false;
	*out_c = buffer[buf_tail];
	buf_tail = (buf_tail + 1) % KBD_BUFFER_SIZE;
	return true;
}

bool keyboard_ctrl_pressed(void)    { return ctrl_pressed; }
bool keyboard_alt_pressed(void)     { return alt_pressed;  }
bool keyboard_shift_pressed(void)   { return shift_pressed;}

static void ps2_wait_input_clear(void) {
  for (int timeout = 100000; timeout > 0; timeout--) {
    if (!(inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL))
      return;
  }
}

static bool ps2_output_has_data(void) {
  return inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL;
}

static void kbd_send_command(uint8_t command) {
  ps2_wait_input_clear();
  outb(PS2_DATA_PORT, command);
}

static void kbd_set_leds(void) {
  uint8_t mask = 0;
  if (caps_lock)
    mask |= KBD_LED_CAPS_LOCK;

  kbd_send_command(KBD_CMD_SET_LEDS);
  ps2_wait_input_clear();
  outb(PS2_DATA_PORT, mask);
}

static void keyboard_callback(struct registers *regs) {
	(void) regs;
  if (!ps2_output_has_data())
    return;
  
  uint8_t scancode = inb(PS2_DATA_PORT);

  if (scancode == KBD_RESP_ACK || scancode == KBD_RESP_RESEND)
    return;

  static bool extended = false;
  if (scancode == SCANCODE_EXTENDED_PREFIX) {
    extended = true;
    return;
  }
  bool was_extended = extended;
  extended = false;

  bool released = scancode & SCANCODE_RELEASE_BIT;
  uint8_t code = scancode & ~SCANCODE_RELEASE_BIT;

  if (was_extended) {
    if (code == SCANCODE_EXT_RCTRL) {
      ctrl_pressed = !released;
    } else if(code == SCANCODE_EXT_RALT) {
      alt_pressed = !released;
    }
  }

  if (code == SCANCODE_LSHIFT || code == SCANCODE_RSHIFT) {
    shift_pressed = !released;
    return;
  }
  if (code == SCANCODE_LSHIFT) {
    ctrl_pressed = !released;
    return;
  }
  if (code == SCANCODE_CAPSLOCK) {
    if (!released) {
      caps_lock = !caps_lock;
      kbd_set_leds();
    }
    return;
  }
  if (released)
    return;

  bool upper = shift_pressed ^ caps_lock;
  char c = upper ? scancode_ascii_shift[code] : scancode_ascii[code];
  if (c == 0)
    return;

  buffer_push(c);
  terminal_putchar(c);


	//bool upper = shift_pressed ^ caps_lock;
	//char c = upper ? scancode_ascii_shift[scancode] : scancode_ascii[scancode];
	//if (c == 0)
	//	return;

	//buffer_push(c);
	//terminal_putchar(c); /* echo so keystrokes are visible immediately */
}

void keyboard_initialize(void) {
	buf_head = buf_tail = 0;
	irq_register_handler(KBD_IRQ, keyboard_callback);
}
