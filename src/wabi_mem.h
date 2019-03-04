/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_types.h"

#define WABI_MEM_FROM 0x0010000000000000
#define WABI_MEM_TO   0x00FFFFFFFFFFFFFF
#define WABI_MEM_HALF ((WABI_MEM_FROM - WABI_MEM_TO) / 2)

void wabi_mem_init(wabi_size_t size);
wabi_word_t *wabi_mem_allocate(wabi_size_t size);
#endif
