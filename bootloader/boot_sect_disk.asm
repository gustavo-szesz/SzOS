disk_load:
  pusha
  ; will be need to setup the value from all registers to be able to read from disk
  push dx

  mov ah, 0x02   ; ah = int 0x13 funci https://stanislavs.org/helppc/int_13-2.html
  mov al, dh     ; al = number sector to read (0x01, 0x80)
  mov cl, 0x02   ; cl sector (0x01, 0x11) 

  mov ch, 0x00   ; ch  = cylinder (0x0...0x3FF, 2 bits uper in cl)
  mov dh, 0x00   ; dh = head number


  int 0x13       ; BIOS INTERRUPT
  jc disk_error  ;if erro


  ;---------------------
  pop dx
  cmp al, dh
  jne sectors_error
  popa
  ret

disk_error:
  mov bx, DISK_ERROR
  call print
  call print_nl
  mov dh, ah ; ah = errorcode
  call print_hex
  jmp disk_loop

sectors_error:
  mov bx, SECTORS_ERROR
  call print

disk_loop:
  jmp $


DISK_ERROR: db "Dis read error", 0
SECTORS_ERROR: db "Incorrect number of sectors read", 0

