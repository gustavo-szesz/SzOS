SECTORS_PER_TRACK equ 18
HEADS_PER_CYLINDER equ 2

disk_load_many:
    pusha
    mov [SECTORS_LEFT], ax
    mov byte [CUR_CYL], 0
    mov byte [CUR_HEAD], 0
    mov byte [CUR_SECT], 2

.loop:
    cmp word [SECTORS_LEFT], 0
    je .done

    mov al, SECTORS_PER_TRACK
    sub al, [CUR_SECT]
    add al, 1
    mov [AVAILABLE], al

    mov ax, [SECTORS_LEFT]
    xor ch, ch
    mov cl, [AVAILABLE]
    cmp ax, cx
    jbe .skip
    mov al, cl
.skip:
    mov [SECTORS_NOW], al

    mov ah, 0x02
    mov al, [SECTORS_NOW]
    mov ch, [CUR_CYL]
    mov cl, [CUR_SECT]
    mov dh, [CUR_HEAD]
    int 0x13
    jc disk_error

    cmp al, [SECTORS_NOW]
    jne sectors_error

    xor ah, ah
    mov al, [SECTORS_NOW]
    sub [SECTORS_LEFT], ax
    shl ax, 9
    add bx, ax

    mov byte [CUR_SECT], 1
    mov al, [CUR_HEAD]
    inc al
    cmp al, HEADS_PER_CYLINDER
    jl .same_cylinder
    mov byte [CUR_HEAD], 0
    inc byte [CUR_CYL]
    jmp .loop
.same_cylinder:
    mov [CUR_HEAD], al
    jmp .loop

.done:
    popa
    ret

disk_error:
    mov bx, DISK_ERROR
    call print
    call print_nl
    mov dh, ah
    call print_hex
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call print

disk_loop:
    jmp $

SECTORS_LEFT: dw 0
CUR_CYL:      db 0
CUR_HEAD:     db 0
CUR_SECT:     db 0
AVAILABLE:    db 0
SECTORS_NOW:  db 0

DISK_ERROR: db "Disk error", 0
SECTORS_ERROR: db "Sector count error", 0
