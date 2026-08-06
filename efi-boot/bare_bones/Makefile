#TARGET = main.efi
#USE_GCC =
#include uefi/Makefile
#OVMF_CODE = /usr/share/edk2/OvmfX64/OVMF_CODE.fd
#OVMF_VARS = OVMF_VARS.fd
#EFI_DIR = EFI/BOOT


#.PHONY: all run clean install

#all: $(TARGET) install

#install: $(TARGET)
##	mkdir -p $(EFI_DIR)
#	cp $(TARGET) $(EFI_DIR)/BOOTX64.EFI


#run: all
#	qemu-system-x86_64 \
#		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
#		-drive if=pflash,format=raw,file=$(OVMF_VARS) \
#		-drive format=raw,file=fat:rw:.
#
#clean:
#	rm -f *.o *.efi *.so *.a
#	rm -rf EFI

CC=i686-elf-gcc
AS=i686-elf-as

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra

.PHONY: all clean iso run

all: szos.bin

boot.o: boot.s
	$(AS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) -c kernel.c -o kernel.o $(CFLAGS)

szos.bin: boot.o kernel.o linker.ld
	$(CC) -T linker.ld -o szos.bin \
		-ffreestanding \
		-nostdlib \
		boot.o kernel.o \
		-lgcc

iso: szos.bin
	mkdir -p isodir/boot
	cp szos.bin isodir/boot/
	grub-mkrescue -o SzOS.iso isodir

run: iso
	qemu-system-x86_64 -cdrom SzOS.iso -boot d

clean:
	rm -f *.o *.bin *.iso
