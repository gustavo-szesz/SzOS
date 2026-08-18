/* ==============================================================
   IDT (Interrupt Descriptor Table) -- x86_64

   IMPORTANTE: em x86_64 (64 bits), cada entrada da IDT tem 16 BYTES,
   não 8 como no x86 de 32 bits. Por isso a struct abaixo tem mais
   campos que a que você tinha escrito -- é o formato exato que a CPU
   espera encontrar na memória, byte a byte.
   ============================================================== */

#include <uefi.h>

/* ---------- Uma entrada da IDT (16 bytes, formato fixo da CPU) ---------- */
typedef struct {
    uint16_t addr_low;      // bits 0-15 do endereço da função handler
    uint16_t sel;           // seletor de segmento de código (do GDT)
    uint8_t  ist;           // Interrupt Stack Table (0 = não usa, por enquanto)
    uint8_t  flags;         // tipo da entrada + nível de privilégio
    uint16_t addr_mid;      // bits 16-31 do endereço
    uint32_t addr_high;     // bits 32-63 do endereço (só existe em 64 bits)
    uint32_t reserved;      // sempre 0, reservado pela spec da Intel
} __attribute__((packed)) idt_entry_t;

/* ---------- A tabela inteira: 256 entradas ---------- */
idt_entry_t idt[256] __attribute__((aligned(16)));

/* ---------- O que a instrução "lidt" espera receber ---------- */
typedef struct {
    uint16_t limit;   // tamanho da tabela em bytes, menos 1
    uint64_t base;    // endereço onde a tabela "idt" está na memória
} __attribute__((packed)) idt_pointer_t;

/* ---------- Protótipos ---------- */
void configure_entry_empty(int number, idt_entry_t *table);
void initiate_idt(void);

/* ==============================================================
   Configura UMA entrada como "vazia" (desativada) por enquanto.
   Isso é seguro: uma entrada vazia não é usada pela CPU.
   ============================================================== */
void configure_entry_empty(int number, idt_entry_t *table) {
    table[number].addr_low  = 0;
    table[number].sel       = 0;
    table[number].ist       = 0;
    table[number].flags     = 0;   // 0 = "não presente", CPU ignora essa entrada
    table[number].addr_mid  = 0;
    table[number].addr_high = 0;
    table[number].reserved  = 0;
}

/* ==============================================================
   Monta a tabela inteira (256 entradas vazias) e avisa a CPU
   onde ela está, usando a instrução de assembly "lidt".
   ============================================================== */
void initiate_idt(void) {
    for (int i = 0; i < 256; i++) {
        configure_entry_empty(i, idt);
    }

    idt_pointer_t ptr;
    ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    ptr.base  = (uint64_t) &idt;

    /* "lidt" é uma instrução de assembly cru: carrega o registrador
       IDTR da CPU com o endereço e tamanho da nossa tabela.
       "m"(ptr) diz ao compilador: "esse operando vem da memória,
       do endereço da variável ptr". */
    asm volatile("lidt %0" : : "m"(ptr));
}