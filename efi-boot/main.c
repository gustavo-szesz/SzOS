#include <uefi.h>
#include "sz.c"

uint32_t *g_framebuffer;
uint32_t  g_pixels_per_line;
int g_ticks = 0;

/* ============================================================
 *         Prototypes
 * ============================================================ */
void draw_logo(void);
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color);
void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color);
void delay(uint64_t quantidade);
void draw_gimp_image(uint32_t *framebuffer, uint32_t screen_x, uint32_t screen_y, uint32_t pixels_per_line);


void program_pit(uint32_t freq_desired);

efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_needed);
efi_physical_address_t my_malloc(int size_in_bytes, efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size);

/* ===========================================================
  IDT
==============================================================*/
typedef struct 
{
   uint16_t addr_low;
   uint16_t sel;
   uint8_t  ist;
   uint8_t  flags;
   uint16_t addr_mid;
   uint32_t addr_high;
   uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

idt_entry_t idt[256] __attribute__((aligned(16)));

typedef struct  
{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed)) idt_pointer_t;

void configure_entry_empty(int number, idt_entry_t *table);
void initiate_idt(void);
void outb(uint16_t port, uint8_t value);
void handler_time(void *frame);

void remap_pic(void);
uint16_t read_current_cs(void);
void delay_real(int ticks_for_wait);
/** ============================================================
        Handler for IDT
* ============================================================ */
void configure_entry_real(int number, idt_entry_t *table, uint64_t address);

void handler_interruption_test(void *frame);
/* ============================================================
 * Count_ += 1
 * ============================================================ */
efi_physical_address_t next_addr_available = 0;

/* ============================================================
 * MAIN
 * ============================================================ */
int main(int argc, char **argv) {
    draw_logo();
    BS->Stall(2000000);

    /* ---------- 1.  o GOP (framebuffer) ---------- */
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;

    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status) || gop == NULL) {
        printf("GOP not available\n");
        return 1;
    }

    uint32_t width           = gop->Mode->Information->HorizontalResolution;
    uint32_t height          = gop->Mode->Information->VerticalResolution;
    uint32_t pixels_per_line = gop->Mode->Information->PixelsPerScanLine;
    uint32_t *framebuffer    = (uint32_t*) gop->Mode->FrameBufferBase;

    /* ---------- 2. ExitBootServices ---------- */
    uintn_t map_size = 0;
    efi_memory_descriptor_t *memory_map = NULL;
    uintn_t map_key;
    uintn_t descriptor_size;
    uint32_t descriptor_version;

    efi_status_t status2 = BS->GetMemoryMap(&map_size, memory_map,
        &map_key, &descriptor_size, &descriptor_version);

    map_size = map_size + (descriptor_size * 2);

    memory_map = (efi_memory_descriptor_t*) malloc(map_size);
    if (memory_map == NULL) {
        printf("malloc failed\n");
        return 1;
    }

    status2 = BS->GetMemoryMap(&map_size, memory_map,
        &map_key, &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status2)) {
        printf("GetMemoryMap 2nd attempt failed\n");
        return 1;
    }

    int how_many_entrys = map_size / descriptor_size;

    status2 = BS->ExitBootServices(IM, map_key);
    if (EFI_ERROR(status2)) {
        return 1;
    }

    /* ============================================================
     *  Ground 0, no libs
     * ============================================================ */

    /* ---------- 3. Test  (my_malloc) ---------- */
    efi_physical_address_t address_test = my_malloc(4096, memory_map, how_many_entrys, descriptor_size);

    uint32_t *pointer_test = (uint32_t*) address_test;
    *pointer_test = 12345;
    uint32_t value_read = *pointer_test;

    if (value_read == 12345) {
        draw_box(framebuffer, 0, 0, 20, pixels_per_line, 0x0000FF00); 
    } else {
        draw_box(framebuffer, 0, 0, 20, pixels_per_line, 0x00FF0000); 
    }

    /* ----------  Animation        ---------- */
    int x = 100, y = 100;
    int dx = 4, dy = 3;
    int box_size = 60;
    uint32_t list_colors[] = {0x00E67E22, 0x003498DB, 0x002ECC71, 0x009B59B6};
    int num_colors = sizeof(list_colors) / sizeof(list_colors[0]);
    int index_atual_color = 0;
    int hit_on_edge;


    initiate_idt();
    g_framebuffer = framebuffer;
    g_pixels_per_line = pixels_per_line;
    configure_entry_real(3, idt, (uint64_t) handler_interruption_test);
    configure_entry_real(32, idt, (uint64_t) handler_time);   

    remap_pic();
    program_pit(100);
    asm volatile("sti");
        
    while (1) {
        /*  */
        for (uint32_t py = 0; py < height; py++) {
            for (uint32_t px = 0; px < width; px++) {
                draw_pixel(framebuffer, px, py, pixels_per_line, 0x00000000);
            }
        }

        draw_gimp_image(framebuffer, 40, 50, pixels_per_line);


        draw_box(framebuffer, x, y, box_size, pixels_per_line, list_colors[index_atual_color]);


        x += dx;
        y += dy;
        hit_on_edge = 0;

        if (x <= 0 || x + box_size >= (int)width) {
            dx = -dx;
            dx += (dx > 0) ? 1 : -1;
            hit_on_edge = 1;
        }
        if (y <= 0 || y + box_size >= (int)height) {
            dy = -dy;
            dy += (dy > 0) ? 1 : -1;
            hit_on_edge = 1;
        }

        if (hit_on_edge == 1) {
            index_atual_color = index_atual_color + 1;
            if (index_atual_color >= num_colors) {
                index_atual_color = 0;
            }
        }

        delay_real(10);
    }

    return 0;
}

// Função para renderizar o gimp_image no framebuffer
void draw_gimp_image(uint32_t *framebuffer, uint32_t screen_x, uint32_t screen_y, uint32_t pixels_per_line) {
    uint32_t img_w = gimp_image.width;
    uint32_t img_h = gimp_image.height;
    uint32_t bpp   = gimp_image.bytes_per_pixel; // Espera-se 4 (RGBA)

    for (uint32_t y = 0; y < img_h; y++) {
        for (uint32_t x = 0; x < img_w; x++) {
            // 1. Calcula o índice base dos bytes deste pixel no array do GIMP
            uint32_t index = (y * img_w + x) * bpp;

            uint8_t r = gimp_image.pixel_data[index + 0];
            uint8_t g = gimp_image.pixel_data[index + 1];
            uint8_t b = gimp_image.pixel_data[index + 2];
            // uint8_t a = gimp_image.pixel_data[index + 3]; // Canal Alpha (transparência)

            // 2. Converte para o formato de pixel 32-bit (0x00RRGGBB)
            uint32_t color = (r << 16) | (g << 8) | b;

            // 3. Desenha no framebuffer se estiver dentro dos limites da tela
            // (Assumindo a mesma lógica que seu draw_pixel faz)
            uint32_t fb_x = screen_x + x;
            uint32_t fb_y = screen_y + y;

            framebuffer[fb_y * pixels_per_line + fb_x] = color;
        }
    }
}

/* ============================================================
 * draw
 * ============================================================ */
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color) {
    *(g_framebuffer + y * pixels_per_line + x) = color;
}

void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color) {
    for (int by = 0; by < size; by++) {
        for (int bx = 0; bx < size; bx++) {
            draw_pixel(g_framebuffer, x + bx, y + by, pixels_per_line, color);
        }
    }
}

void draw_logo(void) {
    const char *logo[] = {
        " >>========================================<< ",
        " ||  .-')      .-') _               .-')   || ",
        " || ( OO ).   (  OO) )             ( OO ). || ",
        " ||(_)---\\_),(_)----. .-'),-----. (_)---\\_)|| ",
        " ||/    _ | |       |( OO'  .-.  '/    _ | || ",
        " ||\\  :` `. '--.   / /   |  | |  |\\  :` `. || ",
        " || '..`''.)(_/   /  \\_) |  |\\|  | '..`''.)|| ",
        " ||.-._)   \\ /   /___  \\ |  | |  |.-._)   \\|| ",
        " ||\\       /|        |  `'  '-'  '\\       /|| ",
        " || `-----' `--------'    `-----'  `-----' || ",
        " >>========================================<< ",
        NULL
    };
    for (int line = 0; logo[line] != NULL; line++) {
        printf("%s\n", logo[line]);
    }
}

/* ============================================================
 * time busy
 * ============================================================ */
void delay(uint64_t quantidade) {
    for (uint64_t i = 0; i < quantidade; i++) {
        /* busy-wait: só gasta tempo de propósito */
    }
}

/* ============================================================
 * malloc memory 
 * ============================================================ */
efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_needed) {
    for (int i = 0; i < how_many_entries; i++) {
        efi_memory_descriptor_t *entry = (efi_memory_descriptor_t*)((uint8_t*)memory_map + (i * descriptor_size));
        if (entry->Type == EfiConventionalMemory) {
            if (entry->NumberOfPages >= pages_needed) {
                return entry->PhysicalStart;
            }
        }
    }
    return 0;
}

efi_physical_address_t my_malloc(int size_in_bytes, efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size) {
    if (next_addr_available == 0) {
        uint64_t pages = (size_in_bytes / 4096) + 1;
        next_addr_available = find_region_for_malloc(memory_map, how_many_entries, descriptor_size, pages);
    }
    efi_physical_address_t address = next_addr_available;
    next_addr_available = next_addr_available + size_in_bytes;
    return address;
}

void configure_entry_empty(int number, idt_entry_t *table) {
    table[number].addr_low  = 0;
    table[number].sel       = 0;
    table[number].ist       = 0;
    table[number].flags     = 0;
    table[number].addr_mid  = 0;
    table[number].addr_high = 0;
    table[number].reserved  = 0;
}


void initiate_idt(void){
    for (int i = 0; i < 256;i++) {
        configure_entry_empty(i, idt);
    }
    
    idt_pointer_t ptr;
    ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    ptr.base = (uint64_t) &idt;
    

    asm volatile("lidt %0" : : "m"(ptr));
 }

void configure_entry_real(int number, idt_entry_t *table, uint64_t address){
    table[number].addr_low = address & 0xFFFF;// 16 bits
    table[number].addr_mid = (address >> 16) & 0xFFFF;
    table[number].addr_high = (address >> 32) & 0xFFFFFFFF;
    table[number].sel      = read_current_cs();
    table[number].ist      = 0;
    table[number].flags    = 0x8E;
    table[number].reserved = 0;
}
__attribute__((interrupt))
void handler_interruption_test(void *frame){
    draw_pixel(g_framebuffer,  930, 0, g_pixels_per_line, 0x00FFFF00);
    (void) frame;
}

void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void remap_pic(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 32);
    outb(0xA1, 40);

    outb(0x21, 4);
    outb(0xA1, 2);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);
}

__attribute__((interrupt))
void handler_time(void *frame) {
    g_ticks = g_ticks + 1;
    outb(0x20, 0x20);
    (void) frame;
}

uint16_t read_current_cs(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return cs;
}

void program_pit(uint32_t freq_desired) {
    int div = 1193181;

    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, (div >> 8) & 0xFF);
}

void delay_real(int ticks_for_wait) {
    int ticks_initial = g_ticks;

    while ((g_ticks - ticks_initial) < ticks_for_wait) {
        continue;
    }
}