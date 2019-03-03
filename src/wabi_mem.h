/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_types.h"
#include "wabi_vm.h"

wabi_word_t* wabi_mem_alloc(wabi_vm_t *vm, wabi_size_t size);
void wabi_mem_init(wabi_vm_t *vm, wabi_size_t size);
void wabi_mem_collect(wabi_vm_t *vm, wabi_word_t* root);
wabi_size_t wabi_mem_size(wabi_vm_t *vm);
wabi_size_t wabi_mem_used(wabi_vm_t *vm);

#define wabi_mem_reference(vm, ptr) ((ptr) - (vm)->space)
#endif
