#include "memory.h"

efi_physical_address_t next_addr_available = 0;

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
