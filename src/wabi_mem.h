/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_value.h"
#include "wabi_vm.h"


void
wabi_mem_init(wabi_vm vm, wabi_size_t size);


void
wabi_mem_free(wabi_vm vm);


wabi_word_t*
wabi_mem_allocate(wabi_vm vm, wabi_size_t size);


wabi_size_t
wabi_mem_used(wabi_vm vm);


wabi_size_t
wabi_mem_total(wabi_vm vm);


void
wabi_mem_collect(wabi_vm vm);

#endif
