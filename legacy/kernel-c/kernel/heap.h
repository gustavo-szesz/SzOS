#ifndef HEAP_H
#define HEAP_H

#include "../cpu/types.h"

void heap_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif // !HEAP_H
