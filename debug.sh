#!/bin/sh
# Same as qemu.sh, but with QEMU's interrupt logging turned on.
# See https://wiki.osdev.org/Troubleshooting and https://wiki.osdev.org/QEMU
#
# -d int          log every interrupt/exception QEMU delivers to the guest,
#                 with a full register dump. This is what shows you the
#                 *first* exception in a triple-fault chain, which is
#                 normally invisible (the machine just silently reboots).
# -d cpu_reset    also log CPU resets (what a triple fault turns into).
# -no-reboot      halt instead of silently rebooting on triple fault.
# -no-shutdown    keep the window open after a shutdown/reset too, so you
#                 can still read the log / attach a debugger.
# -D qemu-int.log write the log to a file instead of stdout, since -d int
#                 is extremely verbose (a dump per interrupt, including
#                 every timer tick).
set -e
. ./iso.sh

rm -f qemu-int.log
qemu-system-x86_64 -cdrom SzOS.iso \
	-d int,cpu_reset -D qemu-int.log \
	-no-reboot -no-shutdown

echo "Interrupt log written to qemu-int.log"
