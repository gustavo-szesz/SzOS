# Sugestões de Melhoria – SzOS (efi-boot)

**Objetivo:** elevar o nível do código C para um padrão mais profissional, legível e manutenível, mantendo a mesma funcionalidade.

---

## 1. Princípios gerais aplicados

- Eliminar **magic numbers** (números soltos).
- Usar `#define` / `enum` com nomes claros.
- Separar responsabilidades (um módulo = uma responsabilidade).
- Nomes de variáveis e funções em inglês consistente (ou português consistente – escolha um e mantenha).
- Comentários que explicam o **porquê**, não o óbvio.
- Funções pequenas e com responsabilidade única.
- Reduzir duplicação (ex.: EOI repetido).

---

## 2. Arquivo de constantes de hardware (recomendado)

Crie um arquivo `include/hardware.h` (ou `ports.h` expandido):

```c
#ifndef HARDWARE_H
#define HARDWARE_H

/* ======================== PIC ======================== */
#define PIC1_COMMAND        0x20
#define PIC1_DATA           0x21
#define PIC2_COMMAND        0xA0
#define PIC2_DATA           0xA1

#define PIC_EOI             0x20
#define PIC_INIT            0x11          /* ICW1 */
#define PIC_ICW4_8086       0x01

#define IRQ_BASE_MASTER     32            /* IRQ 0-7  → vetores 32-39 */
#define IRQ_BASE_SLAVE      40            /* IRQ 8-15 → vetores 40-47 */

/* Máscaras iniciais (habilita timer, teclado e mouse) */
#define PIC1_MASK_DEFAULT   0xF8          /* 11111000 → IRQ 0,1,2 */
#define PIC2_MASK_DEFAULT   0xEF          /* 11101111 → IRQ 12 */

/* ======================== PIT ======================== */
#define PIT_COMMAND         0x43
#define PIT_CHANNEL0        0x40
#define PIT_FREQUENCY_HZ    1193182
#define PIT_MODE_RATE_GEN   0x36          /* Mode 3, binary */

/* ======================== PS/2 ======================== */
#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64

#define PS2_STATUS_OUTPUT   0x01          /* bit 0: output buffer full */
#define PS2_STATUS_INPUT    0x02          /* bit 1: input buffer full */

#define PS2_CMD_ENABLE_AUX  0xA8
#define PS2_CMD_READ_CFG    0x20
#define PS2_CMD_WRITE_CFG   0x60
#define PS2_CMD_WRITE_MOUSE 0xD4

#define MOUSE_CMD_DEFAULTS  0xF6
#define MOUSE_CMD_ENABLE    0xF4

/* ======================== Teclado ======================== */
#define KEY_UP              0x48
#define KEY_DOWN            0x50
#define KEY_LEFT            0x4B
#define KEY_RIGHT           0x4D
#define KEY_SPACE           0x39

/* ======================== IDT ======================== */
#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_INTERRUPT  0x0E
#define IDT_TYPE_INTERRUPT  (IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INTERRUPT) /* 0x8E */

#define PAGE_SIZE           4096

#endif /* HARDWARE_H */
```

---

## 3. Melhorias no PIC / PIT (`pic_timer.c`)

### Antes (trecho atual)
```c
outb(0x20, 0x11);
outb(0xA0, 0x11);
outb(0x21, 32);
...
outb(0x20, 0x20);   // EOI
```

### Depois (pseudocódigo)

```c
#include "hardware.h"
#include "ports.h"

void remap_pic(void)
{
    /* ICW1 – inicia a sequência de inicialização */
    outb(PIC1_COMMAND, PIC_INIT);
    outb(PIC2_COMMAND, PIC_INIT);

    /* ICW2 – base dos vetores de interrupção */
    outb(PIC1_DATA, IRQ_BASE_MASTER);
    outb(PIC2_DATA, IRQ_BASE_SLAVE);

    /* ICW3 – configuração de cascata */
    outb(PIC1_DATA, 0x04);   /* master: slave no IRQ2 */
    outb(PIC2_DATA, 0x02);   /* slave: está no IRQ2 */

    /* ICW4 – modo 8086 */
    outb(PIC1_DATA, PIC_ICW4_8086);
    outb(PIC2_DATA, PIC_ICW4_8086);

    /* Máscaras */
    outb(PIC1_DATA, PIC1_MASK_DEFAULT);
    outb(PIC2_DATA, PIC2_MASK_DEFAULT);
}

void send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void program_pit(uint32_t desired_freq)
{
    uint32_t divisor = PIT_FREQUENCY_HZ / desired_freq;

    outb(PIT_COMMAND, PIT_MODE_RATE_GEN);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

/* Handlers */
__attribute__((interrupt))
void handler_time(void *frame)
{
    g_ticks++;
    send_eoi(0);          /* IRQ0 */
    (void)frame;
}

__attribute__((interrupt))
void handler_keyboard(void *frame)
{
    g_last_key = inb(PS2_DATA_PORT);
    send_eoi(1);          /* IRQ1 */
    (void)frame;
}
```

---

## 4. Melhorias na IDT (`idt.c` / `idt.h`)

### Antes
```c
table[number].flags = 0x8E;
```

### Depois (pseudocódigo)

```c
#include "hardware.h"

void set_idt_entry(uint8_t vector, void (*handler)(void *), uint8_t flags)
{
    uint64_t addr = (uint64_t)handler;

    idt[vector].addr_low  = (uint16_t)(addr & 0xFFFF);
    idt[vector].addr_mid  = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].addr_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].sel      = read_current_cs();
    idt[vector].ist       = 0;
    idt[vector].flags     = flags;
    idt[vector].reserved  = 0;
}

void initiate_idt(void)
{
    for (int i = 0; i < 256; i++)
        set_idt_entry(i, NULL, 0);   /* ou configure_entry_empty */

    /* Handlers reais */
    set_idt_entry(3,  handler_interruption_test, IDT_TYPE_INTERRUPT);
    set_idt_entry(32, handler_time,              IDT_TYPE_INTERRUPT);
    set_idt_entry(33, handler_keyboard,          IDT_TYPE_INTERRUPT);
    set_idt_entry(44, handler_mouse,             IDT_TYPE_INTERRUPT);

    idt_pointer_t ptr = {
        .limit = sizeof(idt) - 1,
        .base  = (uint64_t)&idt
    };
    asm volatile("lidt %0" : : "m"(ptr));
}
```

---

## 5. Melhorias no Mouse (`mouse.c`)

### Antes
```c
outb(0x20, 0x20);
outb(0xA0, 0x20);
write_command(0xA8);
...
```

### Depois (pseudocódigo)

```c
#include "hardware.h"

void wait_input_buffer_clear(void)
{
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT)
        ;   /* espera o buffer de entrada ficar livre */
}

void wait_output_buffer_full(void)
{
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT))
        ;   /* espera dado disponível */
}

void ps2_write_command(uint8_t cmd)
{
    wait_input_buffer_clear();
    outb(PS2_STATUS_PORT, cmd);
}

void ps2_write_data(uint8_t data)
{
    wait_input_buffer_clear();
    outb(PS2_DATA_PORT, data);
}

uint8_t ps2_read_data(void)
{
    wait_output_buffer_full();
    return inb(PS2_DATA_PORT);
}

void activate_mouse(void)
{
    /* Habilita dispositivo auxiliar (mouse) */
    ps2_write_command(PS2_CMD_ENABLE_AUX);

    /* Lê configuração atual */
    ps2_write_command(PS2_CMD_READ_CFG);
    uint8_t config = ps2_read_data();

    config |= 0x02;          /* habilita interrupção do mouse */
    config &= ~0x20;         /* desabilita tradução */

    ps2_write_command(PS2_CMD_WRITE_CFG);
    ps2_write_data(config);

    /* Envia comandos para o mouse */
    ps2_write_command(PS2_CMD_WRITE_MOUSE);
    ps2_write_data(MOUSE_CMD_DEFAULTS);
    ps2_read_data();         /* ACK */

    ps2_write_command(PS2_CMD_WRITE_MOUSE);
    ps2_write_data(MOUSE_CMD_ENABLE);
    ps2_read_data();         /* ACK */
}

__attribute__((interrupt))
void handler_mouse(void *frame)
{
    static int byte_index = 0;
    static int packet[3];

    packet[byte_index++] = inb(PS2_DATA_PORT);

    if (byte_index == 3) {
        int8_t dx = (int8_t)packet[1];
        int8_t dy = (int8_t)packet[2];

        g_mouse_x += dx;
        g_mouse_y -= dy;     /* eixo Y invertido */

        /* Clipping simples */
        if (g_mouse_x < 0) g_mouse_x = 0;
        if (g_mouse_y < 0) g_mouse_y = 0;
        /* (adicione limites de tela quando tiver width/height) */

        byte_index = 0;
    }

    send_eoi(12);            /* IRQ12 */
    (void)frame;
}
```

---

## 6. Melhorias no Alocador (`memory.c`)

### Problemas atuais
- Não alinha o endereço.
- Não marca a região como usada.
- `next_addr_available` é global sem proteção.

### Versão melhorada (pseudocódigo)

```c
#include "hardware.h"

static efi_physical_address_t next_free = 0;

efi_physical_address_t my_malloc(size_t size_in_bytes,
                                 efi_memory_descriptor_t *map,
                                 int entry_count,
                                 uintn_t desc_size)
{
    /* Alinha para 16 bytes (ou use PAGE_SIZE se preferir) */
    size_in_bytes = (size_in_bytes + 15) & ~15;

    if (next_free == 0) {
        uint64_t pages_needed = (size_in_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
        next_free = find_region_for_malloc(map, entry_count, desc_size, pages_needed);
        if (next_free == 0)
            return 0;   /* falha */
    }

    efi_physical_address_t addr = next_free;
    next_free += size_in_bytes;
    return addr;
}
```

---

## 7. Melhorias em Graphics

### Problemas
- `draw_pixel` e `draw_box` recebem `framebuffer` mas ignoram e usam `g_draw_target`.
- Sem clipping.

### Sugestão

```c
void draw_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || y < 0 || x >= (int)g_screen_width || y >= (int)g_screen_height)
        return;

    g_draw_target[y * g_pixels_per_line + x] = color;
}

void draw_box(int x, int y, int size, uint32_t color)
{
    for (int by = 0; by < size; by++)
        for (int bx = 0; bx < size; bx++)
            draw_pixel(x + bx, y + by, color);
}
```

Remova o parâmetro `framebuffer` das funções quando ele não for usado.

---

## 8. Estrutura de pastas sugerida

```
efi-boot/
├── include/
│   ├── hardware.h      ← todos os #defines de ports/flags
│   ├── ports.h
│   ├── pic.h
│   ├── idt.h
│   ├── keyboard.h
│   ├── mouse.h
│   ├── graphics.h
│   └── memory.h
├── src/
│   ├── main.c
│   ├── ports.c
│   ├── pic.c
│   ├── idt.c
│   ├── keyboard.c
│   ├── mouse.c
│   ├── graphics.c
│   └── memory.c
├── Makefile
└── ...
```

---

## 9. `main.c` mais limpo (visão geral)

```c
int main(int argc, char **argv)
{
    /* 1. Fase UEFI */
    draw_logo();
    BS->Stall(2000000);

    if (!init_gop()) return 1;
    if (!exit_boot_services()) return 1;

    /* 2. Fase pós-ExitBootServices */
    g_framebuffer     = ...;
    g_pixels_per_line = ...;
    g_screen_width    = ...;
    g_screen_height   = ...;

    /* 3. Hardware */
    initiate_idt();
    remap_pic();
    program_pit(100);
    mouse_init_state(g_screen_width / 2, g_screen_height / 2);
    activate_mouse();

    asm volatile("sti");

    /* 4. Alocador + backbuffer */
    uint32_t *backbuffer = (uint32_t*)my_malloc(framebuffer_size, ...);
    g_draw_target = backbuffer;

    /* 5. Loop principal */
    while (1) {
        handle_keyboard_input();   /* usa KEY_UP, KEY_SPACE etc. */

        if (!paused) {
            clear_buffer(backbuffer);
            draw_scene(backbuffer);
            present(backbuffer);    /* copy_memory */
            update_animation();
        }

        delay_real(10);
    }

    return 0;
}
```

---

## 10. Checklist de aplicação rápida

- [ ] Criar `hardware.h` com todos os `#define`
- [ ] Substituir todos os `0x20`, `0xA0`, `0x60` etc. pelos macros
- [ ] Criar função `send_eoi(irq)`
- [ ] Trocar `const int UP_KEY = 72` por `#define KEY_UP 0x48`
- [ ] Renomear variáveis confusas (`buffer_rasc` → `backbuffer`, `how_many_entries` → `entry_count`)
- [ ] Adicionar clipping em `draw_pixel`
- [ ] Alinhar alocações no `my_malloc`
- [ ] Separar handlers de teclado e mouse em arquivos próprios (opcional)

---

## 11. Benefícios esperados

| Aspecto              | Antes              | Depois                          |
|----------------------|--------------------|---------------------------------|
| Legibilidade         | Baixa (números)    | Alta (nomes semânticos)         |
| Manutenção           | Difícil            | Fácil de alterar ports/flags    |
| Bugs de digitação    | Mais prováveis     | Reduzidos                       |
| Onboarding           | Precisa de datasheet | Código auto-documentado      |
| Profissionalismo     | Código de aprendizado | Código de projeto real       |

---

**Observação:**  
Todas as mudanças acima são em **pseudocódigo**. Adapte os tipos (`uint8_t`, `uint16_t` etc.) conforme o seu ambiente `posix-uefi` / headers atuais.

Se quiser, no próximo passo posso gerar a versão completa de um arquivo específico (ex.: `pic.c` ou `hardware.h`) já no formato pronto para copiar.  
Basta pedir!
