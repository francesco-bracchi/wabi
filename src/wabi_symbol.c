#define wabi_symbol_c

#include <stddef.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_map.h"

#define WABI_SYMBOL_SIZE 1


wabi_val
wabi_intern(wabi_vm vm, wabi_val bin)
{
  if(! wabi_val_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_val interned = wabi_map_get_raw((wabi_map) vm->symbol_table, bin);
  if(interned) {
    return interned;
  }
  wabi_val symbol = (wabi_val) wabi_mem_allocate(vm, 1);
  if(vm->errno) return NULL;
  *symbol = (wabi_word_t) bin | WABI_TAG_SYMBOL;
  vm->symbol_table = (wabi_word_t*) wabi_map_assoc(vm, vm->symbol_table, bin, symbol);
  if(vm->errno) return NULL;

  return symbol;
}
