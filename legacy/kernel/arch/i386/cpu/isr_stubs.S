.section .text

.macro ISR_NOERRCODE num
.global isr\num
isr\num:
  cli 
  pushl $0
  pushl $\num
  jmp isr_common_stub

.endm

.macro ISR_ERRCODE num
.global isr\num
isr\num:
  cli
  pushl $\num
  jmp isr_common_stub
.endm

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31


.macro IRQ num, remapped
.global irq\num 
irq\num:
  cli 
  pushl $0
  pushl $\remapped
  jmp irq_common_stub
.endm 




IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

# ------------------------------------------------------------------
# Common stubs: save full register state into a `struct registers`,
# switch to the kernel data segment, call the C handler, restore
# everything and `iret` back to whatever was interrupted.
# ------------------------------------------------------------------
isr_common_stub:
	pusha               # edi,esi,ebp,esp,ebx,edx,ecx,eax

	movw %ds, %ax
	pushl %eax          # save the data segment

	movw $0x10, %ax      # kernel data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	pushl %esp          # struct registers* argument
	call isr_handler
	addl $4, %esp

	popl %eax           # restore the original data segment
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	popa
	addl $8, %esp        # pop error code + int_no pushed by the stub
	sti
	iret

irq_common_stub:
	pusha

	movw %ds, %ax
	pushl %eax

	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	pushl %esp
	call irq_handler
	addl $4, %esp

	popl %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	popa
	addl $8, %esp
	sti
	iret 







