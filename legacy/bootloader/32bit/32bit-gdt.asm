gdt_start:
    dd 0x0 ; 4 bytes - limit low
    dd 0x0 ; 4 bytes 

; gdt for code segment
gdt_code:
    dw 0xffff ; segment length
    dw 0x0    ; segment base , bists 0-15
    db 0x0    ; segment base , bits 16-23
    db 10011010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base , bits 24-31

; gdt for data segment base and length
gdt_data:
    dw 0xffff ; segment length
    dw 0x0    ; segment base , bists 0-15
    db 0x0    ; segment base , bits 16-23
    db 10010010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base , bits 24

gdt_end:

; gdt descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bits)
    dd gdt_start                ; address (32 bits)

; constants for latter

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start