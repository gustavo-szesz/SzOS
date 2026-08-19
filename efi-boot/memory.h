#ifndef MEMORY_H
#define MEMORY_H

#include <uefi.h>

efi_physical_address_t find_region_for_malloc(efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size, uint64_t pages_needed);
efi_physical_address_t my_malloc(int size_in_bytes, efi_memory_descriptor_t *memory_map, int how_many_entries, uintn_t descriptor_size);

#endif
