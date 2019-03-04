/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_types.h"

#define WABI_MEM_FROM 0x0010000000000000
#define WABI_MEM_SIZE 0x00000000FFFFFFFF
#define WABI_MEM_TO   (WABI_MEM_FROM + WABI_MEM_SIZE)
#define WABI_MEM_HALF (WABI_MEM_SIZE / 2)

wabi_word_t *wabi_mem_root;

void wabi_mem_init(wabi_size_t size);
wabi_word_t *wabi_mem_allocate(wabi_size_t size);
wabi_word_t wabi_mem_used();
void wabi_mem_collect();
#endif
