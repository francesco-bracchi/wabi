#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"

typedef struct wabi_vm_struct
{
  int errno;
  wabi_size_t mem_size;
  wabi_word_t *mem_from_space;
  wabi_word_t *mem_limit;
  wabi_word_t *mem_scan;
  wabi_word_t *mem_root;
  wabi_word_t *mem_alloc;
  wabi_word_t *symbol_table;
  wabi_word_t *mem_to_space;
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

#endif
