#ifndef wabi_vm_h

#define wabi_vm_h
#include "wabi_types.h"

typedef struct wabi_vm_struct {
  wabi_word_t* mem_space;
  wabi_word_t* mem_space_limit;
  wabi_word_t* mem_alloc;
  wabi_word_t* mem_from_space;
  wabi_word_t* mem_scan;
  int errno;
} wabi_vm_t;

wabi_vm_t* wabi_vm_new();

int wabi_vm_init(wabi_vm_t* vm, wabi_size_t size);

#endif
