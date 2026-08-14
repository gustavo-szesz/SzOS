#!/bin/sh
# Boots SzOS paused, waiting for GDB to attach over the QEMU gdbstub.
# See https://wiki.osdev.org/Kernel_Debugging#Debugging_with_GDB
#
# -s     shorthand for -gdb tcp::1234
# -S     freeze the CPU at the very first instruction (don't run until
#        GDB sends 'continue')
set -e
. ./iso.sh

qemu-system-x86_64 -cdrom SzOS.iso -s -S &
QEMU_PID=$!

cat <<EOF

QEMU is paused, waiting for GDB on localhost:1234 (PID $QEMU_PID).
In another terminal:

    gdb kernel/szos.kernel
    (gdb) set architecture i386:x86-64
    (gdb) target remote localhost:1234
    (gdb) break kernel_main
    (gdb) continue

NOTE: "set architecture i386:x86-64" is required even though this is a
32-bit kernel. qemu-system-x86_64's gdbstub always reports the target as
i386:x86-64 (the CPU itself supports long mode), but GDB auto-selects
plain "i386" from the 32-bit ELF you loaded. That mismatch breaks the
register-read ('g') packet and silently drops the session. Forcing the
architecture to match what QEMU reports fixes it.

EOF

wait $QEMU_PID
