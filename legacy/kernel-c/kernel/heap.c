#include "heap.h"
#define HEAP_START 0x100000 // 1MB
#define HEAP_SIZE  0x400000 // 4MB heap 

static uint32_t heap_current;
static uint32_t heap_end;

void heap_init(){
  heap_current = HEAP_START;
  heap_end = HEAP_START + HEAP_SIZE;
}

void* kmalloc(uint32_t size){
  size = (size + 3) & ~3;

  if (heap_current + size > heap_end) {
    for (;;) ;
    
  }

  void* ptr = (void*)heap_current;
  heap_current += size;
  return ptr;
}

void kfree(void* ptr){ (void)ptr; }
