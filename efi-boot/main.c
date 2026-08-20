#include <uefi.h>
#include "include/idt.h"
#include "include/ports.h"
#include "include/pic_timer.h"
#include "include/graphics.h"
#include "include/memory.h"
#include "include/font.h"
#include "include/mouse.h"

const int UP_KEY = 72;
const int DOWN_KEY = 80;
const int LEFT_KEY = 75;
const int RIGHT_KEY = 77;
const int ESPACE    = 57;

int paused = 0;
void handler_mouse(void *frame);
void activate_mouse();

int main(int argc, char **argv) {
    draw_logo();
    BS->Stall(2000000);

    

    /* ---------- 1. GOP (framebuffer) ---------- */
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

    int how_many_entries = map_size / descriptor_size;

    status2 = BS->ExitBootServices(IM, map_key);
    if (EFI_ERROR(status2)) {
        return 1;
    }

    /* ============================================================
     * A partir daqui: sem printf, sem malloc, sem BS->Stall
     * ============================================================ */

    g_framebuffer     = framebuffer;
    g_pixels_per_line = pixels_per_line;

    /* ---------- 3. Teste do alocador caseiro ---------- */
    efi_physical_address_t address_test = my_malloc(4096, memory_map, how_many_entries, descriptor_size);
    uint32_t *pointer_test = (uint32_t*) address_test;
    *pointer_test = 12345;
    uint32_t value_read = *pointer_test;

    if (value_read == 12345) {
        draw_box(framebuffer, 0, 0, 20, pixels_per_line, 0x0000FF00);
    } else {
        draw_box(framebuffer, 0, 0, 20, pixels_per_line, 0x00FF0000);
    }
    

    /* ---------- 4. IDT + PIC + PIT ---------- */
    initiate_idt();
    configure_entry_real(3, idt, (uint64_t) handler_interruption_test);
    configure_entry_real(32, idt, (uint64_t) handler_time);
    configure_entry_real(33, idt, (uint64_t) handler_keyboard);

    configure_entry_real(44, idt, (uint64_t) handler_mouse);
    mouse_init_state(width / 2, height / 2);
    activate_mouse();

    remap_pic();
    program_pit(100); /* 100 ticks/segundo */
    asm volatile("sti");

    /* ---------- 5. Animação ---------- */
    int x = 100, y = 100;
    int dx = 4, dy = 3;
    int box_size = 60;
    uint32_t list_colors[] = {0x00E67E22, 0x003498DB, 0x002ECC71, 0x009B59B6};
    int num_colors = sizeof(list_colors) / sizeof(list_colors[0]);
    int index_atual_color = 0;
    int hit_on_edge;
    int size_buffer = height *( g_pixels_per_line * 4);
    uint32_t *buffer_rasc = (uint32_t*) my_malloc(size_buffer, memory_map,
                              how_many_entries,  descriptor_size);

    g_draw_target = buffer_rasc;

    while (1) {
        if (g_last_key == ESPACE) {
            paused = (paused == 1)? 0 : 1;
            g_last_key = 0;
        } else if (g_last_key == UP_KEY) {
            y -= 10;
            g_last_key = 0;
        } else if (g_last_key == LEFT_KEY){
            x -= 10;
            g_last_key = 0;
        } else if (g_last_key == RIGHT_KEY){
            x += 10;
            g_last_key = 0;
        } else if (g_last_key == DOWN_KEY){   
            y += 10;
            g_last_key = 0;
        }

        if (x < 0){ x = 0; }
        if (x + box_size > (int)width){x = width - box_size;}
        if (y < 0 ){y=0;}
        if (y + box_size > (int)height){ y = height - box_size;}

        if (paused == 0) {

        
        for (uint32_t py = 0; py < height; py++) {
            for (uint32_t px = 0; px < width; px++) {
                draw_pixel(framebuffer, px, py, pixels_per_line, 0x00000000);
            }
        }

        draw_gimp_image(buffer_rasc, 40, 50, pixels_per_line);
        draw_box(buffer_rasc, x, y, box_size, pixels_per_line, list_colors[index_atual_color]);

        draw_box(buffer_rasc, g_mouse_x, g_mouse_y, 8, g_pixels_per_line , 0x00FFFFFF);

        draw_number((uint32_t) g_ticks, 10, 10, 0x00FFFFFF);

        copy_memory(framebuffer, buffer_rasc, size_buffer);

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
    }

    return 0;
}
