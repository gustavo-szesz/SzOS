ARCH = x86_64
TARGET = main.efi
#USE_GCC = 1          ← comenta ou apaga essa linha

OVMF_CODE = /usr/share/edk2/OvmfX64/OVMF_CODE.fd
OVMF_VARS = OVMF_VARS.fd
EFI_DIR = EFI/BOOT

include uefi/Makefile

install: $(TARGET)
	mkdir -p $(EFI_DIR)
	cp $(TARGET) $(EFI_DIR)/BOOTX64.EFI

run: all install
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=$(OVMF_VARS) \
		-drive format=raw,file=fat:rw:.

clean:
	rm -f *.o *.efi *.so *.a
	rm -rf EFI