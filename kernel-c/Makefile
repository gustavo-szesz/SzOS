C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJS = $(C_SOURCES:.c=.o)

CC = /home/szesz/opt/cross/bin/i686-elf-gcc
GDB = /home/szesz/opt/cross/bin/i686-elf-gdb

CFLAGS = -g

os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > os-image.bin

kernel.bin: boot/kernel_entry.o $(OBJS)
	i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

# debug
kernel.elf: boot/kernel_entry.o $(OBJS)
	i686-elf-ld -o $@ -Ttext 0x1000 $^

run: os-image.bin
	qemu-system-x86_64 -fda os-image.bin

debug: kernel.elf
	qemu-system-x86_64 -fda os-image.bin &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

#rules for wildcards
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o