#include <uefi.h>

void draw_logo(void);
void draw_pixel(uint32_t *framebuffer, int x, int y, uint32_t pixels_per_line, int color);
void delay(uint64_t quantidade);

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
    int[] list_colors = [0x00E67E22, 0x003498DB, 0x002ECC71, 0x009B59B6];
    int index_atual_color 


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
                draw_pixel(framebuffer, x + bx, y + by, pixels_per_line, 0x00E67E22);
            }
        }

        x += dx;
        y += dy;

        if (x <= 0 || x + box_size >= (int)width)  dx = -dx;
        if (y <= 0 || y + box_size >= (int)height) dy = -dy;

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