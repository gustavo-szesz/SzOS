#include <uefi.h>

efi_physical_address_t next_addrs_avaliable = 0;

void draw_logo(void);
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color);
void delay(uint64_t quantidade);
efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_registry);

int main(int argc, char **argv) {
    draw_logo();
    BS->Stall(2000000);

    // --- pegar o GOP ---
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

    // --- ExitBootServices ---
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

    status2 = BS->ExitBootServices(IM, map_key);
    if (EFI_ERROR(status2)) {
        return 1;
    }
    int x = 100, y = 100;
    int dx = 4, dy = 3;
    int box_size = 60;
    int list_colors[] = {0x00E67E22, 0x003498DB, 0x002ECC71, 0x009B59B6};
    int index_atual_color = 0;
    int hit_on_edge; 


    while (1) {
        // apaga a tela (preto)
        for (uint32_t py = 0; py < height; py++) {
            for (uint32_t px = 0; px < width; px++) {
                draw_pixel(framebuffer, px, py, pixels_per_line, 0x00000000);
            }
        }

        // desenha o quadrado
        for (int by = 0; by < box_size; by++) {
            for (int bx = 0; bx < box_size; bx++) {
                draw_pixel(framebuffer, x + bx, y + by, pixels_per_line, list_colors[index_atual_color]);
            }
        }

        x += dx;
        y += dy;


        if (x <= 0 || x + box_size >= (int)width) {
            dx += (dx > 0) ? 1 : -1;
            hit_on_edge = 1;}
            
        if (y <= 0 || y + box_size >= (int)height) {
            dx += (dx > 0) ? 1 : -1;
            hit_on_edge = 1;
        }


        if (hit_on_edge == 1){
            index_atual_color = index_atual_color + 1;
            if (index_atual_color >= sizeof(list_colors)){
                index_atual_color = 0;
            }
        }

        delay(30000000);
    }

    return 0;
}

void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color) {
    *(framebuffer + y * pixels_per_line + x) = color;
}

void delay(uint64_t quantidade) {
    for (uint64_t i = 0; i < quantidade; i++) {
        // busy-wait: só gasta tempo de propósito
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

// 118:} efi_memory_descriptor_t;
// 119-
// 120-typedef struct {
// 121-    uint64_t    Signature;
// 122-    uint32_t    Revision;
// 123-    uint32_t    HeaderSize;
// 124-    uint32_t    CRC32;
// 125-    uint32_t    Reserved;
// 126-} efi_table_header_t;
// 528:    EfiConventionalMemory,

efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_registry){
    for (int i = 0; i < how_many_entries; i++) {
        efi_memory_descriptor_t *entry = (efi_memory_descriptor_t*)((uint8_t*)memory_map + (i * descriptor_size));
        if(entry->Type == EfiConventionalMemory){
            if (entry->NumberOfPages >= pages_registry) {
                return entry->PhysicalStart;
            }
        }
    }
    return 0; 

}
efi_physical_address_t my_malloc(int size_in_bytes, efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size){
    if (next_addrs_avaliable == 0) {
        uint64_t pages = (size_in_bytes / 4096) + 1;
        next_addrs_avaliable = find_region_for_malloc(memory_map, how_many_entries, descriptor_size, pages);
    }
    efi_physical_address_t address = next_addrs_avaliable;
    next_addrs_avaliable = next_addrs_avaliable + size_in_bytes;
    return address;
}