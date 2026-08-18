#include <uefi.h>

/* ============================================================
 *         Prototypes
 * ============================================================ */
void draw_logo(void);
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color);
void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color);
void delay(uint64_t quantidade);

efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_needed);
efi_physical_address_t my_malloc(int size_in_bytes, efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size);

/* ===========================================================
  IDT
==============================================================*/
struct idt_entry
{
   uint16_t addr_l;
   uint16_t sel;
   uint8_t  nulls;
   uint8_t  attrib;
   uint16_t addr_h;
} __attribute__((packed));

struct idt_entry idt[256] __attribute__((aligned(4)));

struct idt_pointer 
{
    unsigned short limit;
    unsigned int base;
}__attribute__((packed));

int configure_entry_empty(int number, int *table);

void initiate_idt(){
    // for (int i = 0; i >= 256;i++) {
    //     configure_entry_empty(int number, int *)
    // }
}

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

    while (1) {
        /*  */
        for (uint32_t py = 0; py < height; py++) {
            for (uint32_t px = 0; px < width; px++) {
                draw_pixel(framebuffer, px, py, pixels_per_line, 0x00000000);
            }
        }

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

        delay(30000000);
    }

    return 0;
}

/* ============================================================
 * draw
 * ============================================================ */
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color) {
    *(framebuffer + y * pixels_per_line + x) = color;
}

void draw_box(uint32_t *framebuffer, int x, int y, int size, uint32_t pixels_per_line, uint32_t color) {
    for (int by = 0; by < size; by++) {
        for (int bx = 0; bx < size; bx++) {
            draw_pixel(framebuffer, x + bx, y + by, pixels_per_line, color);
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



