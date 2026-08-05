TARGET = main.efi
USE_GCC =
include uefi/Makefile
OVMF_CODE = /usr/share/edk2/OvmfX64/OVMF_CODE.fd
OVMF_VARS = OVMF_VARS.fd
EFI_DIR = EFI/BOOT


.PHONY: all run clean install

all: $(TARGET) install

install: $(TARGET)
	mkdir -p $(EFI_DIR)
	cp $(TARGET) $(EFI_DIR)/BOOTX64.EFI


run: all
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=$(OVMF_VARS) \
		-drive format=raw,file=fat:rw:.

clean:
	rm -f *.o *.efi *.so *.a
	rm -rf EFI
