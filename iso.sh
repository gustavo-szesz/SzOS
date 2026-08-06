#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp $SYSROOT/boot/szos.kernel isodir/boot/szos.kernel
cat >isodir/boot/grub/grub.cfg <<EOF
menuentry "szos" {
	multiboot /boot/szos.kernel
}
EOF
grub-mkrescue -o SzOS.iso isodir
