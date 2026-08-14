#include <stddef.h>

#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <stdint.h>

#include "../cpu/io.h"
#include "../cpu/irq.h"
#include "../cpu/isr.h"

/* --- i8042 PS/2 controller ports (see OSDev: "8042" PS/2 Controller) --- */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_STATUS_OUTPUT_FULL 0x01 /* bit 0: 1 = data waiting to be read  */
#define PS2_STATUS_INPUT_FULL  0x02 /* bit 1: 1 = don't write yet, busy    */

/* --- PS/2 Keyboard device commands (see OSDev: PS/2 Keyboard) --- */
#define KBD_CMD_SET_LEDS 0xED
#define KBD_RESP_ACK     0xFA
#define KBD_RESP_RESEND  0xFE

#define KBD_LED_SCROLL_LOCK 0x01
#define KBD_LED_NUM_LOCK    0x02
#define KBD_LED_CAPS_LOCK   0x04

#define KBD_IRQ 1

#define KBD_BUFFER_SIZE 256

static char buffer[KBD_BUFFER_SIZE];
static volatile size_t buf_head = 0; /* next write position */
static volatile size_t buf_tail = 0; /* next read position   */

static bool shift_pressed = false;
static bool ctrl_pressed  = false;
static bool alt_pressed   = false;
static bool caps_lock     = false;

/* US QWERTY, scan code set 1 (translated by the 8042 from the keyboard's
 * native set 2 -- this is what you get by default on real hardware and
 * in QEMU/Bochs). Index = scan code (make code), unshifted. */
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

/* Scan code set 1, non-extended */
#define SC_LSHIFT      0x2A
#define SC_RSHIFT      0x36
#define SC_LCTRL       0x1D
#define SC_LALT        0x38
#define SC_CAPSLOCK    0x3A
#define SC_RELEASE_BIT 0x80
#define SC_EXTENDED_PREFIX 0xE0

/* Extended (0xE0-prefixed) scan codes we care about. Everything else
 * extended (media keys, ACPI power keys, etc.) is simply ignored. */
#define SC_EXT_RCTRL   0x1D
#define SC_EXT_RALT    0x38

/* --- tiny ring buffer, filled by the IRQ handler, drained by the caller --- */
static void buffer_push(char c) {
	size_t next = (buf_head + 1) % KBD_BUFFER_SIZE;
	if (next == buf_tail) /* full: drop the oldest byte to make room */
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

bool keyboard_ctrl_pressed(void)  { return ctrl_pressed; }
bool keyboard_alt_pressed(void)   { return alt_pressed; }
bool keyboard_shift_pressed(void) { return shift_pressed; }

/* --- talking to the 8042 controller / the keyboard device ---
 * Per OSDev, before writing to port 0x60 or 0x64 you must wait for the
 * "input buffer" status bit (0x64, bit 1) to clear; before reading port
 * 0x60 you should check the "output buffer full" bit (0x64, bit 0). */
static void ps2_wait_input_clear(void) {
	// for (int timeout = 100000; timeout > 0; timeout--) {
	// 	if (!(inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL))
	// 		return;
	// }
	for (int t = 100000; t > 0; t--) {
        if (!(inb(0x64) & 0x02))
            return;
    }
}

static int ps2_wait_output_full(void) {
    for (int t = 100000; t > 0; t--) {
        if (inb(0x64) & 0x01)
            return 1;   /* tem dado */
    }
    return 0;           /* timeout */
}

static bool ps2_output_has_data(void) {
	return inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL;
}

/* Sends a one-byte command to the keyboard device itself (not the 8042
 * controller). Used here for 0xED (Set LEDs). The ACK/Resend response
 * comes back through the normal IRQ1 line, so we don't block on it here
 * -- keyboard_callback() just knows to swallow 0xFA/0xFE. */
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

/* TEMP DEBUG PROBE — enabled unconditionally so we don't depend on any
 * build-flag plumbing (config.sh was clobbering CFLAGS regardless of what
 * was exported). Remove this #define once the keyboard is confirmed
 * working, or once you've read the debug digit. */
#define KBD_DEBUG_PROBE 1

static void keyboard_callback(struct registers *regs) {
	(void) regs;
    static uint8_t p = '0';
    ((volatile uint16_t *)0xB8000)[0] = (uint16_t)p | (0x4F << 8);
    p = (p == '9') ? '0' : p + 1;
    if (inb(0x64) & 1)
        inb(0x60);

// 	(void) regs;
// 	static uint8_t probe = '0';
// 	((volatile uint16_t *) 0xB8000)[79] = (uint16_t) probe | (0x4F << 8);
// 	probe = (probe == '9') ? '0' : probe + 1;

// 	if (inb(0x64) & 0x01)
//         inb(0x60);

// #ifdef KBD_DEBUG_PROBE
// 	/* TEMP: proves the handler is actually running on every keypress,
// 	 * independent of scancode decoding. Writes directly to VGA memory
// 	 * at the top-right corner so it doesn't disturb terminal_row/column. */
// 	static uint8_t probe2 = '0';
// 	((volatile uint16_t *) 0xB8000)[79] = (uint16_t) probe2 | (0x4F << 8);
// 	probe2 = (probe2 == '9') ? '0' : probe2 + 1;
// #endif

// 	if (!ps2_output_has_data())
// 		return; /* spurious IRQ, nothing to read */

// 	uint8_t scancode = inb(PS2_DATA_PORT);

// 	/* Keyboard's own replies to our commands -- not a key event. */
// 	if (scancode == KBD_RESP_ACK || scancode == KBD_RESP_RESEND)
// 		return;

// 	/* 0xE0 prefixes a second byte identifying keys that don't exist on
// 	 * the original XT layout (right Ctrl/Alt, arrows, Home/End/Del,
// 	 * Insert, Page Up/Down, the Windows keys...). We only special-case
// 	 * the ones we act on (right Ctrl/Alt) and otherwise just swallow
// 	 * the following byte so it isn't misread as a random ASCII key. */
// 	static bool extended = false;
// 	if (scancode == SC_EXTENDED_PREFIX) {
// 		extended = true;
// 		return;
// 	}
// 	bool was_extended = extended;
// 	extended = false;

// 	bool released = scancode & SC_RELEASE_BIT;
// 	uint8_t code = scancode & ~SC_RELEASE_BIT;

// 	if (was_extended) {
// 		if (code == SC_EXT_RCTRL) {
// 			ctrl_pressed = !released;
// 		} else if (code == SC_EXT_RALT) {
// 			alt_pressed = !released;
// 		}
// 		/* arrows / Home / End / Del / Insert / PgUp / PgDn / GUI keys:
// 		 * no ASCII mapping yet, silently ignored. */
// 		return;
// 	}

// 	if (code == SC_LSHIFT || code == SC_RSHIFT) {
// 		shift_pressed = !released;
// 		return;
// 	}
// 	if (code == SC_LCTRL) {
// 		ctrl_pressed = !released;
// 		return;
// 	}
// 	if (code == SC_LALT) {
// 		alt_pressed = !released;
// 		return;
// 	}
// 	if (code == SC_CAPSLOCK) {
// 		if (!released) { /* toggle on make code only, not on release */
// 			caps_lock = !caps_lock;
// 			kbd_set_leds();
// 		}
// 		return;
// 	}
// 	if (released)
// 		return; /* release of a regular key: nothing to do */

// 	bool upper = shift_pressed ^ caps_lock;
// 	char c = upper ? scancode_ascii_shift[code] : scancode_ascii[code];
// 	if (c == 0)
// 		return; /* unmapped key (F-keys, Num Lock, ...) */

// 	buffer_push(c);
// 	//terminal_putchar(c); /* echo so keystrokes are visible immediately */
}

void keyboard_initialize(void) {
	buf_head = buf_tail = 0;
    irq_register_handler(KBD_IRQ, keyboard_callback);
// buf_head = buf_tail = 0;

//     /* Disable devices */
//     ps2_wait_input_clear();
//     outb(0x64, 0xAD);

//     ps2_wait_input_clear();
//     outb(0x64, 0xA7);

//     /* Flush output buffer */
//     for (int i = 0; i < 100; i++) {
//         if (!(inb(0x64) & 0x01))
//             break;
//         inb(0x60);
//     }

//     /* Read config */
//     ps2_wait_input_clear();
//     outb(0x64, 0x20);

//     uint8_t config = 0x00;
//     if (ps2_wait_output_full())
//         config = inb(0x60);

//     config |= 0x01;     /* keyboard IRQ on */
//     config &= (uint8_t)~0x02;  /* mouse IRQ off */

//     /* Write config */
//     ps2_wait_input_clear();
//     outb(0x64, 0x60);
//     ps2_wait_input_clear();
//     outb(0x60, config);

//     /* Enable keyboard port */
//     ps2_wait_input_clear();
//     outb(0x64, 0xAE);

//     /* Enable scanning (opcional) */
//     ps2_wait_input_clear();
//     outb(0x60, 0xF4);

//     irq_register_handler(KBD_IRQ, keyboard_callback);

	// buf_head = buf_tail = 0;

    // /* Disable devices */
    // ps2_wait_input_clear();
    // outb(0x64, 0xAD);           /* disable keyboard */

    // ps2_wait_input_clear();
    // outb(0x64, 0xA7);           /* disable mouse */

    // /* Flush output buffer */
    // while (inb(0x64) & 0x01)
    //     inb(0x60);

    // /* Read config byte */
    // ps2_wait_input_clear();
    // outb(0x64, 0x20);           /* "read config" */

    // while (!(inb(0x64) & 0x01)) /* wait until data ready */
    //     ;

    // uint8_t config = inb(0x60);

    // config |= 0x01;             /* enable keyboard IRQ */
    // config &= ~0x02;            /* disable mouse IRQ */

    // /* Write config byte */
    // ps2_wait_input_clear();
    // outb(0x64, 0x60);           /* "write config" */
    // ps2_wait_input_clear();
    // outb(0x60, config);

    // /* Enable keyboard port */
    // ps2_wait_input_clear();
    // outb(0x64, 0xAE);

    // /* Enable scanning (optional) */
    // ps2_wait_input_clear();
    // outb(0x60, 0xF4);

    // irq_register_handler(KBD_IRQ, keyboard_callback);
}
