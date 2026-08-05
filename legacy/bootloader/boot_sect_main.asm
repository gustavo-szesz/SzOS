[org 0x7c00]
  mov bp, 0x8000
  mov sp, bp

  mov bx, 0x9000
  mov dh, 2

  call disk_load

  mov dx, [0x9000]
  call print_string_pm

  call print_nl

  mov dx, [0x9000 + 512]
  call print_hex

  jmp $

%include "boot_sect_print.asm"
%include "boot_sect_print_hex.asm"
%include "boot_sect_disk.asm"
%include "32bit/32bit-print.asm"

times 510-($-$$) db 0
dw 0xaa55


; boot sector = sector 1 of cyl 0 of head 0 hdd 0
; from now, sector 2...
times 256 dw 0xdada ; sector 2 = 512 bytes
times 256 dw 0xface ; sector 3 = 512 bytes
