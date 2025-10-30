loop:
    jmp loop

 times 510-($-$$) db 0
; Magic number
dw 0xaa55 
