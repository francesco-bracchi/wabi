/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_types.h"
#include "wabi_vm.h"

wabi_word_t* wabi_mem_malloc(wabi_vm_t *vm, wabi_size_t size);
void wabi_mem_init(wabi_vm_t *vm, wabi_size_t size);
void wabi_mem_collect(wabi_vm_t *vm, wabi_word_t* size);

#endif
