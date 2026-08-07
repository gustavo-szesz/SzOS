#ifndef ARCH_I386_CPU_ISR_H
#define ARCH_I386_CPU_ISR_H

#include <stdint.h>

struct registers {
  uint32_t ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed));

typedef void (*isr_t)(struct registers *regs);

void isr_install(void);

void isr_register_handler(uint8_t n, isr_t handler);

void isr_handler(struct registers *regs);

#endif // !ARCH_I386_CPU_ISR_H

